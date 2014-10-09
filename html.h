/*
 * html.h - HTML templates for photo gallery
 *
 * Copyright (C) 2014		Andrew Clayton <andrew@digital-domain.net>
 *
 * Licensed under the GNU General Public License V2
 * See COPYING
 */

#ifndef _HTML_H_
#define _HTML_H_

#define P_HTML_HEAD	"<!DOCTYPE html\n\
     PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\"\n\
     \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">\n\
<html xmlns=\"http://www.w3.org/1999/xhtml\" xml:lang=\"en\" lang=\"en\">\n\
<head>\n\
<title>%s (%d/%d)</title>\n\
<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" />\n\
  <meta name=\"Generator\" content=\"yagp - Yet Another Gallery Program\" />\n\
  <link href=\"theme/layout.css\" rel=\"stylesheet\" type=\"text/css\" />\n\
  <link href=\"theme/style.css\" rel=\"stylesheet\" type=\"text/css\" />\n\
</head>\n"

#define P_HTML_BODY	"<body>\n\
  <div id=\"wrapper\">\n\
    <div id=\"main\">\n\
\n\
      <h1 class=\"image-title\">\n\
        %s</h1>\n\
\n\
      <ul class=\"navigation-bar\">\n\
        <li class=\"index\">\n\
          <a href=\"%s\" title=\"Go back to the index\">\n\
            <span>Go back to the index</span>\n\
          </a>\n\
        </li>\n"

#define P_N_IMAGE	"<li class=\"next-image\">\n\
          <a href=\"%s.html\" title=\"View the next image\">\n\
            <span>View the next image</span>\n\
          </a>\n\
        </li>\n"

#define P_C_IMAGE	"<li class=\"current-image\">\n\
          <span>%d / %d</span>\n\
        </li>\n"

#define P_P_IMAGE	"<li class=\"previous-image\">\n\
          <a href=\"%s.html\" title=\"View the previous image\">\n\
            <span>View the previous image</span>\n\
          </a>\n\
        </li>\n"

#define P_N_V		"<li class=\"void\">\n\
        </li>\n"

#define P_N_LAST	"<li class=\"last\">\n\
        </li>\n\
      </ul>\n\n"

#define P_IMG		"<div class=\"image-preview\">\n\
      <a href=\"../%s\"><img src=\"../previews/%s.medium.jpeg\" alt=\"../previews/%s.medium.jpeg\" width=\"%d\" height=\"%d\" class=\"preview\" /></a></div>\n\
\n\
      <div class=\"image-comment\">\n\
        %s</div>\n\
      </div>\n\
  </div>\n"

#define P_HTML_END	"<div id=\"footer\">\n\
    <div id=\"footer-2\">\n\
      </div>\n\
  </div>\n\
</body>\n\
</html>\n"

#define HTML_HEAD	"<!DOCTYPE html\n\
     PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\"\n\
     \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">\n\
<html xmlns=\"http://www.w3.org/1999/xhtml\" xml:lang=\"en\" lang=\"en\">\n\
<head>\n\
<title>%s (%d/%d)</title>\n\
<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" />\n\
  <meta name=\"Generator\" content=\"yagp - Yet Another Gallery Program\" />\n\
  <link href=\"theme/layout.css\" rel=\"stylesheet\" type=\"text/css\" />\n\
  <link href=\"theme/style.css\" rel=\"stylesheet\" type=\"text/css\" />\n\
</head>\n"

#define HTML_BODY	"<body>\n\
  <div id=\"wrapper\">\n\
    <div id=\"main\">\n\
    \n\
      <h1 class=\"index-title\">\n\
        %s</h1>\n\
      \n\
      <ul class=\"navigation-bar\">\n"

#define N_PAGE		"<li class=\"next-page\">\n\
          <a href=\"%s\" title=\"View the next page\">\n\
            <span>View the next page</span>\n\
          </a>\n\
        </li>\n"

#define C_PAGE		"<li class=\"current-page\">\n\
          <span>%d / %d</span>\n\
        </li>\n"

#define P_PAGE		"<li class=\"previous-page\">\n\
          <a href=\"%s\" title=\"View the previous page\">\n\
            <span>View the previous page</span>\n\
          </a>\n\
        </li>\n"

#define P_V		"<li class=\"void\">\n\
        </li>\n"

#define N_LAST		"<li class=\"last\">\n\
        </li>\n\
      </ul>\n\n"

#define THUMB_CT	"<div class=\"thumbnail-container\">\n\
        <div class=\"thumbnail-top\">\n\
        </div>\n\
        <div class=\"thumbnail-center\">\n\
          <div class=\"thumbnail-left\">\n\
            <div class=\"thumbnail-right\">\n\
              <div class=\"thumbnail-image-container\">\n\
                <a class=\"thumbnail-image\" href=\"%s.html\" title=\"Click to view the image\"><img src=\"../thumbnails/%s.small.jpeg\" alt=\"../thumbnails/%s.small.jpeg\" width=\"%d\" height=\"%d\" /></a>\n\
              </div>\n\
            </div>\n\
          </div>\n\
        </div>\n\
        <div class=\"thumbnail-bottom\">\n\
        </div>\n\
      </div> <!-- thumbnail-container -->\n\
\n\
      <div class=\"caption-container\">\n\
        <div class=\"property-row\">\n\
	    <span class=\"property-value\" title=\"Description\">%s</span>\n\
          </div>\n\
\n\
        <div class=\"property-row\">\n\
	    <span class=\"property-value\" title=\"Date\">%s</span>\n\
          </div>\n\
\n\
        </div> <!-- caption-container -->\n\
        </div> <!-- thumbnail-caption-container -->\n\
\n"

#define HTML_END	"</div>\n\n\
    </div>\n\
  </div>\n\
  <div id=\"footer\">\n\
    <div id=\"footer-2\">\n\
      </div>\n\
  </div>\n\
</body>\n\
</html>\n"

#endif /* _HTML_H_ */
