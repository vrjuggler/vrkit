# vrkit is (C) Copyright 2005-2011
#    by Allen Bierbaum, Aron Bierbuam, Patrick Hartling, and Daniel Shipton
#
# This file is part of vrkit.
#
# vrkit is free software; you can redistribute it and/or modify it under the
# terms of the GNU Lesser General Public License as published by the Free
# Software Foundation; either version 2 of the License, or (at your option)
# any later version.
#
# vrkit is distributed in the hope that it will be useful, but WITHOUT ANY
# WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
# more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

default: html

docs: html chunk-html pdf
install-docs: install-html install-chunk-html install-pdf

NAME=		user_guide
XML_FILES=	$(NAME).xml

XSLT_TOOL=	Saxon

#SVG_IMAGES=	
#IMAGES=		$(SVG_IMAGES:%.svg=%.png)

# Fill these in!!  Together, they make up the installation prefix.
webroot=	$(HOME)/public_html
instdir=	user_guide

prefix=		$(webroot)/$(instdir)
INSTALL_DIRS=	images
#INSTALL_FILES=	$(webroot)/base_style.css

#NEED_DB_IMAGES=	1

#$(NAME).html: $(IMAGES)
#$(NAME).pdf: $(IMAGES)

#CLOBBER_FILES=	$(IMAGES)

include ../docbook.mk

docs.tar: docs
	tar --exclude \*.svn -cvf $@ user_guide.html user_guide.pdf user_guide
