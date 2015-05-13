/*
 * yagp - Yet Another Gallery Program
 *
 * Copyright (C) 2014 - 2015	Andrew Clayton <andrew@digital-domain.net>
 *
 * Licensed under the GNU General Public License V2
 * See COPYING
 */

#define _GNU_SOURCE		/* dirent.d_type / DT_* */
#define _XOPEN_SOURCE	700	/* getopt(3), scandir(3) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <dirent.h>
#include <limits.h>
#include <math.h>

#include <libexif/exif-data.h>

#include <wand/magick_wand.h>

#include "html.h"

#define LIGHT_UP_AND_RIGHT	"\342\224\224"	/* └ */
#define LIGHT_DOWN_AND_RIGHT	"\342\224\214"	/* ┌ */

#define THUMB_W		160
#define THUMB_W_P	 90
#define THUMB_H		120
#define PREVIEW_W	650
#define PREVIEW_W_P	450
#define PREVIEW_H	488
#define PREVIEW_H_P	600

#define PORTRAIT	0
#define LANDSCAPE	1

#define IMGS_PER_PAGE	imgs_per_page
#define IMGS_PER_ROW	imgs_per_row

#define IMGS_ALLOC_SZ		  50
#define ENC_HTML_ALLOC_SZ	4096

#define ALBUM_TITLE	album_title

static int imgs_per_page = 16;
static int imgs_per_row = 4;

static int nr_images;
static int nr_pages;

static char (*images)[NAME_MAX + 2];	/* +2 for P or L and \0 */
static char *album_title;

/*
 * Encode certain characters of the image description to avoid
 * generating duff html.
 */
static char *encode_chars(const char *src)
{
	char *encoded_html = malloc(ENC_HTML_ALLOC_SZ);
	size_t alloc = ENC_HTML_ALLOC_SZ;

	encoded_html[0] = '\0';
	for ( ; *src != '\0'; src++) {
		if (strlen(encoded_html) + 7 > alloc) {
			encoded_html = realloc(encoded_html,
					alloc + ENC_HTML_ALLOC_SZ);
			alloc += ENC_HTML_ALLOC_SZ;
		}
		switch (*src) {
		case '&':
			strcat(encoded_html, "&amp;");
			break;
		case '<':
			strcat(encoded_html, "&lt;");
			break;
		case '>':
			strcat(encoded_html, "&gt;");
			break;
		case '"':
			strcat(encoded_html, "&quot;");
			break;
		case '\'':
			strcat(encoded_html, "&#x27;");
			break;
		case '/':
			strcat(encoded_html, "&#x2F;");
			break;
		default:
			strncat(encoded_html, src, 1);
		}
	}

	return encoded_html;
}

/*
 * Used in scandir(3) to sort the images by mtime, where two images have
 * the same mtime it will use strcoll(3) to compare the image names and
 * sort on that.
 */
static int mtimesort(const struct dirent **a, const struct dirent **b)
{
	struct stat asb;
	struct stat bsb;

	stat((*a)->d_name, &asb);
	stat((*b)->d_name, &bsb);

	if (asb.st_mtime < bsb.st_mtime)
		return -1;
	else if (asb.st_mtime > bsb.st_mtime)
		return 1;
	else
		return strcoll((*a)->d_name, (*b)->d_name);
}

/*
 * Quick sanity check if a file is likely to be a JPEG by file
 * extension.
 */
static int is_image_file(const struct dirent *d)
{
	char *ext;

	if ((d->d_type != DT_REG && d->d_type != DT_UNKNOWN) ||
	    d->d_name[0] == '.')
		return 0;
	if (d->d_type == DT_UNKNOWN) {
		struct stat sb;

		stat(d->d_name, &sb);
		if (!S_ISREG(sb.st_mode))
			return 0;
	}

	ext = strrchr(d->d_name, '.');
	if (!ext)
		return 0;
	ext++;

	if (strcasecmp(ext, "jpg") == 0 ||
	    strcasecmp(ext, "jpeg") == 0)
		return 1;
	else
		return 0;
}

static char *exif_date_to_date(char *date)
{
	date[4] = '-';
	date[7] = '-';

	return date;
}

static void create_html_index(void)
{
	FILE *fp = fopen("index.html", "w");

	fprintf(fp, INDEX_HTML);
	fclose(fp);
}

static void create_preview_html(const char *page, int image_no,
				const char *description)
{
	FILE *fp;
	char name[PATH_MAX];
	char *img;
	int width = PREVIEW_W;
	int height = PREVIEW_H;

	img = images[image_no];
	snprintf(name, sizeof(name), "html/%s.html", img + 1);
	fp = fopen(name, "w");
	fprintf(fp, P_HTML_HEAD, ALBUM_TITLE, image_no + 1, nr_images);
	fprintf(fp, P_HTML_BODY, ALBUM_TITLE, page);

	if (image_no + 1 == nr_images) {
		fprintf(fp, P_N_V);
		fprintf(fp, P_C_IMAGE, image_no + 1, nr_images);
		fprintf(fp, P_P_IMAGE, images[image_no - 1] + 1);
	} else if (image_no + 1 == 1) {
		fprintf(fp, P_N_IMAGE, images[image_no + 1] + 1);
		fprintf(fp, P_C_IMAGE, image_no + 1, nr_images);
		fprintf(fp, P_N_V);
	} else {
		fprintf(fp, P_N_IMAGE, images[image_no + 1] + 1);
		fprintf(fp, P_C_IMAGE, image_no + 1, nr_images);
		fprintf(fp, P_P_IMAGE, images[image_no - 1] + 1);
	}
	fprintf(fp, P_N_LAST);

	if (img[0] == 'P') {
		width = PREVIEW_W_P;
		height = PREVIEW_H_P;
	}
	img++;

	fprintf(fp, P_IMG, img, img, img, width, height, description);
	fprintf(fp, P_HTML_END);
	fclose(fp);
}

static void create_html(void)
{
	int i;
	int image_no = 0;

	printf("\nGenerating HTML with %d image(s) spread over %d page(s)\n",
			nr_images, nr_pages);

	for (i = 0; i < nr_pages; i++) {
		FILE *fp;
		char name[PATH_MAX];
		char ppage[NAME_MAX + 1];
		char npage[NAME_MAX + 1];
		char *img;
		int page_no = i + 1;
		int j;

		snprintf(name, sizeof(name), "html/page%04d.html", page_no);
		fp = fopen(name, "w");
		fprintf(fp, HTML_HEAD, ALBUM_TITLE, page_no, nr_pages);
		fprintf(fp, HTML_BODY, ALBUM_TITLE);

		snprintf(npage, sizeof(npage), "page%04d.html", page_no + 1);
		snprintf(ppage, sizeof(ppage), "page%04d.html", page_no - 1);
		if (page_no == nr_pages) {
			fprintf(fp, P_V);
			fprintf(fp, C_PAGE, page_no, nr_pages);
			fprintf(fp, P_PAGE, ppage);
		} else if (page_no == 1) {
			fprintf(fp, N_PAGE, npage);
			fprintf(fp, C_PAGE, page_no, nr_pages);
			fprintf(fp, P_V);
		} else {
			fprintf(fp, N_PAGE, npage);
			fprintf(fp, C_PAGE, page_no, nr_pages);
			fprintf(fp, P_PAGE, ppage);
		}
		fprintf(fp, N_LAST);
		fprintf(fp, THUMB_LST);

		for (j = 0; j < IMGS_PER_PAGE; j++) {
			int width = THUMB_W;
			int height = THUMB_H;
			char date[32] = "\0";
			char desc[512] = "\0";
			char *enc_desc;
			ExifData *ed;
			ExifEntry *ee;

			if (image_no == nr_images)
				break;
			if (j && j % IMGS_PER_ROW == 0)
				fprintf(fp, THUMB_NEW_ROW);
			fprintf(fp, THUMB_CELL);
			img = images[image_no];
			if (img[0] == 'P')
				width = THUMB_W_P;
			img++;

			ed = exif_data_new_from_file(img);
			ee = exif_content_get_entry(ed->ifd[EXIF_IFD_EXIF],
					EXIF_TAG_DATE_TIME_ORIGINAL);
			exif_entry_get_value(ee, date, sizeof(date));
			ee = exif_content_get_entry(ed->ifd[EXIF_IFD_EXIF],
					EXIF_TAG_USER_COMMENT);
			exif_entry_get_value(ee, desc, sizeof(desc));

			enc_desc = encode_chars(desc);
			fprintf(fp, THUMB_CT, img, img, img, width, height,
					enc_desc, exif_date_to_date(date));
			create_preview_html(name + 5, image_no, enc_desc);
			image_no++;
			exif_data_unref(ed);
			free(enc_desc);
		}
		fprintf(fp, HTML_END);
		fclose(fp);
	}
	create_html_index();
}

static void create_preview(const MagickWand *wand, const char *image,
			   const struct stat *sb)
{
	MagickWand *cwand;
	char name[PATH_MAX];
	unsigned long width = PREVIEW_W;
	unsigned long height = PREVIEW_H;
	int err;
	struct stat psb;
	struct timeval times[2];

	snprintf(name, sizeof(name), "previews/%s.medium.jpeg", image);
	err = stat(name, &psb);
	if (!err && sb->st_mtime <= psb.st_mtime) {
		printf("%sSkipping preview for %s\n", LIGHT_UP_AND_RIGHT,
				name);
		return;
	}

	printf("%sCreating preview for %s\n", LIGHT_UP_AND_RIGHT, image);
	cwand = CloneMagickWand(wand);
	if (MagickGetImageHeight(cwand) > MagickGetImageWidth(cwand)) {
		width = PREVIEW_W_P;
		height = PREVIEW_H_P;
	}
	MagickResizeImage(cwand, width, height, LanczosFilter, 1.0);

	MagickWriteImage(cwand, name);
	DestroyMagickWand(cwand);

	times[0].tv_sec = sb->st_atime;
	times[0].tv_usec = 0;
	times[1].tv_sec = sb->st_mtime;
	times[1].tv_usec = 0;
	utimes(name, times);
}

static int create_thumbnail(const MagickWand *wand, const char *image,
			    struct stat *sb)
{
	MagickWand *cwand = CloneMagickWand(wand);
	char name[PATH_MAX];
	unsigned long width = THUMB_W;
	unsigned long height = THUMB_H;
	int orient = LANDSCAPE;
	int err;
	struct stat tsb;
	struct timeval times[2];

	if (MagickGetImageHeight(cwand) > MagickGetImageWidth(cwand)) {
		width = THUMB_W_P;
		orient = PORTRAIT;
	}

	snprintf(name, sizeof(name), "thumbnails/%s.small.jpeg", image);
	stat(image, sb);
	err = stat(name, &tsb);
	if (!err && sb->st_mtime <= tsb.st_mtime) {
		printf("%sSkipping thumbnail for %s\n", LIGHT_DOWN_AND_RIGHT,
				name);
		goto out;
	}

	printf("%sCreating thumbnail for %s\n", LIGHT_DOWN_AND_RIGHT, image);
	MagickResizeImage(cwand, width, height, LanczosFilter, 1.0);

	MagickWriteImage(cwand, name);

	times[0].tv_sec = sb->st_atime;
	times[0].tv_usec = 0;
	times[1].tv_sec = sb->st_mtime;
	times[1].tv_usec = 0;
	utimes(name, times);

out:
	DestroyMagickWand(cwand);

	return orient;
}

static void process_images(int (*sortfunc)
			   (const struct dirent **a, const struct dirent **b))
{
	struct dirent **namelist;
	int i;
	int entries;

	entries = scandir(".", &namelist, is_image_file, sortfunc);
	if (entries < 0) {
		perror("scandir");
		exit(EXIT_FAILURE);
	}

	for (i = 0; i < entries; i++) {
		MagickWand *wand;
		MagickPassFail status;
		int orient;
		const char *d_name = namelist[i]->d_name;
		struct stat sb;

		wand = NewMagickWand();
		status = MagickReadImage(wand, d_name);
		if (status != MagickPass)
			goto skip;

		if (nr_images % IMGS_ALLOC_SZ == 0)
			images = realloc(images, nr_images * sizeof(*images) +
					IMGS_ALLOC_SZ * sizeof(*images));

		orient = create_thumbnail(wand, d_name, &sb);
		create_preview(wand, d_name, &sb);

		snprintf(images[nr_images], sizeof(images[nr_images]), "%s%s",
				(orient == PORTRAIT) ? "P" : "L", d_name);
		nr_images++;
skip:
		DestroyMagickWand(wand);
		free(namelist[i]);
	}
	free(namelist);

	nr_pages = ceil((double)nr_images / IMGS_PER_PAGE);
}

static void disp_usage(void)
{
	printf("Usage: yagp [-s <name|mtime>] [-p images per page] "
			"[-r images per row]\n"
			"\t    <-t title>\n");
	printf("\n-s is what order to process the images in, either by name "
			"(default) or by mtime\n"
			"images per page should be wholly divisible by images "
			"per row\n");
	exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
	int opt;
	int (*sortfunc)(const struct dirent **a, const struct dirent **b) =
		alphasort;

	while ((opt = getopt(argc, argv, "p:r:t:s:h")) != -1) {
		switch (opt) {
		case 's':
			if (strcmp(optarg, "mtime") == 0)
				sortfunc = mtimesort;
			else if (strcmp(optarg, "name") == 0)
				sortfunc = alphasort;
			else
				disp_usage();
			break;
		case 't':
			album_title = optarg;
			break;
		case 'p':
			imgs_per_page = atoi(optarg);
			break;
		case 'r':
			imgs_per_row = atoi(optarg);
			break;
		default:
			disp_usage();
		}
	}
	if (!album_title)
		disp_usage();
	if (imgs_per_page % imgs_per_row)
		disp_usage();

	InitializeMagick(*argv);

	mkdir("thumbnails", 0777);
	mkdir("previews", 0777);
	mkdir("html", 0777);

	process_images(sortfunc);
	create_html();

	DestroyMagick();
	free(images);

	exit(EXIT_SUCCESS);
}
