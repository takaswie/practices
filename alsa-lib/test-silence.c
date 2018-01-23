#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include <inttypes.h>

#include <getopt.h>
#include <limits.h>

#include <alsa/asoundlib.h>

#define MAX_DESC_COUNT		16
#define MAX_FRAME_COUNT		32
#define MAX_DST_OFFSET		64

#define ARRAY_SIZE(array)	(sizeof(array)/sizeof(array[0]))

static void check(snd_pcm_channel_area_t *descriptors,
		  snd_pcm_uframes_t dst_offset, unsigned int descriptor_count,
		  snd_pcm_uframes_t frame_count, int phys_width,
		  uint64_t *silent_samples, uint8_t *buf_end, const char *label,
		  int *err)
{
	unsigned int phys_bytes;
	uint8_t *addr;
	unsigned int bytes_to_first_sample;
	unsigned int bytes_between_two_samples;
	snd_pcm_uframes_t f;
	int ch;

	phys_bytes = phys_width / 8;

	for (ch = 0; ch < descriptor_count; ++ch) {
		/*
		 * Each descriptor has information for a corresponding
		 * channel.
		 */
		addr = (uint8_t *)descriptors[ch].addr;
		bytes_to_first_sample = descriptors[ch].first / 8;
		bytes_between_two_samples = descriptors[ch].step / 8;

		/*
		 * Each descriptor includes its position to a first sample for
		 * a corresponding channel.
		 */
		addr += bytes_to_first_sample;

		/* The value of 'dst_offset' has frame count. */
		addr += bytes_between_two_samples * dst_offset;

		/*
		 * Each descriptor includes width between samples for
		 * consecutive two frames.
		 */
		for (f = 0; f < frame_count; ++f) {
			/* A case of my mis-programming. */
			assert(addr < buf_end);

			if (memcmp(addr, silent_samples, phys_bytes)) {
				printf("  phys_width %u, "
				       "dst_offset %03" PRIu64 ", "
				       "ch %03u/(%03u), "
				       "frames %03" PRIu64 "/(%03" PRIu64 "), "
				       "%s\n",
				       phys_width,
				       dst_offset,
				       ch, descriptor_count,
				       f, frame_count,
				       label);

				*err = -EIO;
				return;
			}
			addr += bytes_between_two_samples;
		}
	}
}

/*
 * MEMO: In a case of 'korg1212', alignment of data sample on page frame dedicated to DMA
 * transmission is quite similar to a case of usual 'interleaved' buffer, except for
 * each data frame includes unused channel to consists of fixed number of samples in a
 * data frame.
 *
 * $ cd linux/
 * $ git grep -A20 'const struct snd_pcm_ops' sound/ | grep \\.ioctl | \
 *   grep -v snd_pcm_lib_ioctl
 */
static const char *prepare_for_i(snd_pcm_channel_area_t *descriptors,
				 unsigned int descriptor_count,
				 snd_pcm_uframes_t frame_count, uint8_t *buf,
				 int phys_width)
{
	int ch;

	for (ch = 0; ch < descriptor_count; ++ch) {
		descriptors[ch].addr = buf;
		descriptors[ch].first = ch * phys_width;
		descriptors[ch].step = descriptor_count * phys_width;
	}

	return "interleaved";
}

/*
 * MEMO: In cases of 'HDSP'/'HDSPM'/'RME9652', alignment of data sample on page frame
 * dedicated to DMA transmission is quite similar to a case of usual 'non-interleaved'
 * buffer, except for its order of area for samples of each channel.
 */
static const char *prepare_for_n(snd_pcm_channel_area_t *descriptors,
				 unsigned int descriptor_count,
				 snd_pcm_uframes_t frame_count, uint8_t *buf,
				 int phys_width)
{
	int ch;

	for (ch = 0; ch < descriptor_count; ++ch) {
		descriptors[ch].addr = buf;
		descriptors[ch].first = ch * phys_width * frame_count;
		descriptors[ch].step = phys_width;
	}

	return "non-interleaved";
}

static void test(snd_pcm_channel_area_t *descriptors,
		 snd_pcm_uframes_t dst_offset, unsigned int descriptor_count,
		 snd_pcm_uframes_t frame_count, snd_pcm_format_t sample_format,
		 uint8_t *buf, uint8_t *buf_end, int *err)
{
	const char *(*prep_funcs[])(snd_pcm_channel_area_t *descriptors,
				    unsigned int descriptor_count,
				    snd_pcm_uframes_t frame_count, uint8_t *buf,
				    int phys_width) = {
		prepare_for_i,
		prepare_for_n,
	};
	int phys_width;
	uint64_t silent_samples;
	const char *label;

	/* Ensure physical width of the data sample in byte unit. */
	phys_width = snd_pcm_format_physical_width(sample_format);
	if (phys_width < 0 || phys_width / 8 == 0) {
		*err = ENOENT;
		return;
	}

	silent_samples = snd_pcm_format_silence_64(sample_format);

	for (int i = 0; i < sizeof(prep_funcs)/sizeof(prep_funcs[i]); ++i) {
		label = prep_funcs[i](descriptors, descriptor_count,
				      frame_count, buf, phys_width);

		*err = snd_pcm_areas_silence(descriptors, dst_offset,
					     descriptor_count, frame_count,
					     sample_format);
		if (*err < 0)
			break;

		check(descriptors, dst_offset, descriptor_count, frame_count,
		      phys_width, &silent_samples, buf_end, label, err);
		if (*err < 0)
			break;
	}
}

static void try(snd_pcm_channel_area_t *descriptors,
	       unsigned int descriptor_count, snd_pcm_format_t sample_format,
	       uint8_t *buf, uint8_t *buf_end, int fd,
	       unsigned int max_frame_count, unsigned int max_dst_offset,
	       int *err)
{
	snd_pcm_uframes_t frame_count;
	snd_pcm_uframes_t dst_offset;
	ssize_t len;
	size_t size;

	for (frame_count = 1; frame_count <= max_frame_count; ++frame_count) {
		for (dst_offset = 0; dst_offset <= max_dst_offset; ++dst_offset) {
			/* Fill random value. */
			size = buf_end - buf;
			len = read(fd, buf, size);
			if (len != size) {
				if (len < 0)
					*err = -errno;
				else
					*err = -EIO;
				return;
			}

			test(descriptors, dst_offset, descriptor_count,
			     frame_count, sample_format, buf, buf_end, err);
			if (*err < 0)
				break;
		}

		if (*err < 0)
			break;
	}
}

static unsigned int parse_literal(const char *str, int *err)
{
	long val;
	char *endptr;

	val = strtol(str, &endptr, 10);
	if (val == LONG_MIN || val == LONG_MAX) {
		*err = -errno;
		return 0;
	}
	if (*endptr != '\0') {
		*err = -EINVAL;
		return 0;
	}
	if (val < 0 || val > UINT_MAX) {
		*err = -EINVAL;
		return 0;
	}

	return (unsigned int)val;
}

int main(int argc, char *argv[])
{
	static const char *s_opts = "d:c:s:f:";
	static const struct option l_opts[] = {
		{"dst-offset",		1, 0, 'd'},
		{"frame-count",		1, 0, 'c'},
		{"samples-per-frame",	1, 0, 's'},
		{"sample-format",	1, 0, 'f'},
		{NULL,			0, 0, 0},
	};
	static const snd_pcm_format_t skip_formats[] = {
		SND_PCM_FORMAT_IMA_ADPCM,
		SND_PCM_FORMAT_MPEG,
		SND_PCM_FORMAT_GSM,
		SND_PCM_FORMAT_SPECIAL,
		SND_PCM_FORMAT_G723_24,
		SND_PCM_FORMAT_G723_24_1B,
		SND_PCM_FORMAT_G723_40,
		SND_PCM_FORMAT_G723_40_1B,
	};
	int l_idx;
	snd_pcm_format_t sample_format, first_sample_format, last_sample_format;
	int fd;
	const char *label;
	snd_pcm_channel_area_t *descriptors;
	unsigned int max_dst_offset;
	unsigned int max_frame_count;
	unsigned int max_descriptor_count;
	unsigned int descriptor_count;
	size_t size;
	uint8_t *buf, *buf_end;
	int i;
	int err = 0;

	max_dst_offset = MAX_DST_OFFSET;
	max_frame_count = MAX_FRAME_COUNT;
	max_descriptor_count = MAX_DESC_COUNT;
	first_sample_format = SND_PCM_FORMAT_S8;
	last_sample_format = SND_PCM_FORMAT_LAST;

	optind = 0;
	opterr = 0;
	while (1) {
		char c = getopt_long(argc, argv, s_opts, l_opts, &l_idx);
		if (c < 0) {
			break;
		} else if (c == 'd') {
			unsigned int val = parse_literal(optarg, &err);
			if (val >= MAX_DST_OFFSET) {
				err = -EINVAL;
				break;
			}
			max_dst_offset = val;
		} else if (c == 'c') {
			 unsigned int val = parse_literal(optarg, &err);
			if (val == 0 || val >= MAX_FRAME_COUNT) {
				err = -EINVAL;
				break;
			}
			max_frame_count = val;
		} else if (c == 's') {
			unsigned int val = parse_literal(optarg, &err);
			if (val == 0 || val >= MAX_DESC_COUNT) {
				err = -EINVAL;
				break;
			}
			max_descriptor_count = val;
		} else if (c == 'f') {
			snd_pcm_format_t f = snd_pcm_format_value(optarg);
			if (f == SND_PCM_FORMAT_UNKNOWN) {
				err = -EINVAL;
				break;
			}
			first_sample_format = last_sample_format = f;
		} else {
			continue;
		}

		if (err < 0) {
			printf("Fail to parse '%s'(%c) option: %s\n",
			       l_opts[l_idx].name, c, strerror(-err));
			return EXIT_FAILURE;
		}
	}

	fd = open("/dev/urandom", O_RDONLY);
	if (fd < 0) {
		printf("open(2): %s\n", strerror(errno));
		return EXIT_FAILURE;
	}

	/* As of 2018, ALSA PCM interface supports PCM samples up to 64 bit. */
	size = MAX_DESC_COUNT * 8 * (MAX_FRAME_COUNT + MAX_DST_OFFSET);
	buf = malloc(size);
	if (!buf) {
		printf("calloc(3): %s\n", strerror(ENOMEM));
		close(fd);
		return EXIT_FAILURE;
	}
	buf_end = buf + size;

	descriptors = calloc(MAX_DESC_COUNT, sizeof(*descriptors));
	if (!descriptors) {
		printf("calloc(3): %s\n", strerror(ENOMEM));
		free(buf);
		close(fd);
		return EXIT_FAILURE;
	}

	for (sample_format = first_sample_format;
	     sample_format <= last_sample_format; ++sample_format) {
		label = snd_pcm_format_name(sample_format);

		/* For empty slots. */
		if (!label)
			continue;

		/* Some of sample formats can not be handled for this aim. */
		for (i = 0; i < ARRAY_SIZE(skip_formats); ++i) {
			if (sample_format == skip_formats[i])
				break;
		}
		if (i != ARRAY_SIZE(skip_formats))
			continue;

		printf("testcase: %s\n", label);

		for (descriptor_count = 1;
		     descriptor_count <= max_descriptor_count;
		     ++descriptor_count) {
			try(descriptors, descriptor_count, sample_format, buf,
			    buf_end, fd, max_frame_count, max_dst_offset,
			    &err);
			if (err < 0)
				break;
		}

		if (err < 0) {
			printf("  failed due to %s.\n",
			       strerror(-err));
			break;
		}
	}

	free(descriptors);
	free(buf);
	close(fd);

	if (err < 0)
		return EXIT_FAILURE;

	return EXIT_SUCCESS;
}
