/*
 * yagp - Yet Another Gallery Program
 *
 * Copyright (C) 2014		Andrew Clayton <andrew@digital-domain.net>
 *
 * Licensed under the GNU General Public License V2
 * See COPYING
 */

#define _BSD_SOURCE		/* DT_ */
#define _XOPEN_SOURCE		/* optarg */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/stat.h>
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

#define IMGS_PER_PAGE	16
#define IMGS_PER_ROW	 4

#define IMGS_ALLOC_SZ	50

#define ALBUM_TITLE	album_title

static int nr_images;
static int nr_pages;
static int imgs_allocd_sz = IMGS_ALLOC_SZ;

static char (*images)[NAME_MAX + 2];	/* +2 for P or L and \0 */
static char *album_title;

/*
 * Quick sanity check if a file is likely to be a JPEG by file
 * extension.
 */
static bool is_image_file(const char *image)
{
	char *ext;

	ext = rindex(image, '.');
	if (!ext)
		return false;
	ext++;

	if (strcasecmp(ext, "jpg") == 0 ||
	    strcasecmp(ext, "jpeg") == 0)
		return true;
	else
		return false;
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

	fprintf(fp, "<html>\n");
	fprintf(fp, "<head>\n");
	fprintf(fp, "\t<meta http-equiv=\"refresh\" content=\"0; url=html/page0001.html\">\n");
	fprintf(fp, "</head>\n");
	fprintf(fp, "</html>\n");

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
			char date[32];
			char desc[512] = "\0";
			ExifData *ed;
			ExifEntry *ee;

			if (image_no == nr_images)
				break;
			if (j && j % IMGS_PER_ROW == 0)
				fprintf(fp, "</tr>\n<tr class=\"tr_index\">\n");
			fprintf(fp, "<td class=\"td_index\">\n<div class=\"thumbnail-caption-container\">\n");
			img = images[image_no];
			if (img[0] == 'P')
				width = THUMB_W_P;
			img++;

			ed = exif_data_new_from_file(img);
			ee = exif_content_get_entry(ed->ifd[EXIF_IFD_0],
					EXIF_TAG_DATE_TIME);
			exif_entry_get_value(ee, date, sizeof(date));
			ee = exif_content_get_entry(ed->ifd[EXIF_IFD_EXIF],
					EXIF_TAG_USER_COMMENT);
			exif_entry_get_value(ee, desc, sizeof(desc));

			fprintf(fp, THUMB_CT, img, img, img, width, height,
					desc, exif_date_to_date(date));
			create_preview_html(name + 5, image_no, desc);
			image_no++;
			exif_data_unref(ed);
		}
		fprintf(fp, HTML_END);
		fclose(fp);
	}
	create_html_index();
}

static void create_preview(const MagickWand *wand, const char *image)
{
	MagickWand *cwand;
	char name[PATH_MAX];
	unsigned long width = PREVIEW_W;
	unsigned long height = PREVIEW_H;
	struct stat sb;
	time_t orig_mtime;

	snprintf(name, sizeof(name), "previews/%s.medium.jpeg", image);
	stat(image, &sb);
	orig_mtime = sb.st_mtime;
	stat(name, &sb);
	if (orig_mtime < sb.st_mtime) {
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
}

static int create_thumbnail(const MagickWand *wand, const char *image)
{
	MagickWand *cwand = CloneMagickWand(wand);
	char name[PATH_MAX];
	unsigned long width = THUMB_W;
	unsigned long height = THUMB_H;
	struct stat sb;
	time_t orig_mtime;
	int orient = LANDSCAPE;

	if (MagickGetImageHeight(cwand) > MagickGetImageWidth(cwand)) {
		width = THUMB_W_P;
		orient = PORTRAIT;
	}

	snprintf(name, sizeof(name), "thumbnails/%s.small.jpeg", image);
	stat(image, &sb);
	orig_mtime = sb.st_mtime;
	stat(name, &sb);
	if (orig_mtime < sb.st_mtime) {
		printf("%sSkipping thumbnail for %s\n", LIGHT_DOWN_AND_RIGHT,
				name);
		goto out;
	}

	printf("%sCreating thumbnail for %s\n", LIGHT_DOWN_AND_RIGHT, image);
	MagickResizeImage(cwand, width, height, LanczosFilter, 1.0);

	MagickWriteImage(cwand, name);
out:
	DestroyMagickWand(cwand);

	return orient;
}

static void process_images(void)
{
	DIR *dir;
	struct dirent *dt;

	images = malloc(IMGS_ALLOC_SZ * sizeof(*images));

	dir = opendir(".");
	while ((dt = readdir(dir)) != NULL) {
		MagickWand *wand;
		MagickPassFail status;
		int orient;

		if ((dt->d_type != DT_REG && dt->d_type != DT_UNKNOWN) ||
		    dt->d_name[0] == '.')
			continue;
		if (dt->d_type == DT_UNKNOWN) {
			struct stat sb;

			stat(dt->d_name, &sb);
			if (!S_ISREG(sb.st_mode))
				continue;
		}
		if (!is_image_file(dt->d_name))
			continue;

		wand = NewMagickWand();
		status = MagickReadImage(wand, dt->d_name);
		if (status != MagickPass)
			goto skip;

		if (nr_images == imgs_allocd_sz) {
			images = realloc(images, nr_images * sizeof(*images) +
					IMGS_ALLOC_SZ * sizeof(*images));
			imgs_allocd_sz += IMGS_ALLOC_SZ;
		}

		orient = create_thumbnail(wand, dt->d_name);
		create_preview(wand, dt->d_name);

		snprintf(images[nr_images], sizeof(images[nr_images]), "%s%s",
				(orient == PORTRAIT) ? "P" : "L", dt->d_name);
		nr_images++;
skip:
		DestroyMagickWand(wand);
	}
	closedir(dir);

	nr_pages = ceil((double)nr_images / IMGS_PER_PAGE);
}

int main(int argc, char *argv[])
{
	int opt;

	while ((opt = getopt(argc, argv, "t:")) != -1) {
		switch (opt) {
		case 't':
			album_title = optarg;
			break;
		default:
			printf("Usage: yagp <-t title>\n");
			exit(EXIT_FAILURE);
		}
	}
	if (!album_title) {
		printf("Usage: yagp <-t title>\n");
		exit(EXIT_FAILURE);
	}

	InitializeMagick(*argv);

	mkdir("thumbnails", 0777);
	mkdir("previews", 0777);
	mkdir("html", 0777);

	process_images();
	create_html();

	DestroyMagick();

	exit(EXIT_SUCCESS);
}
