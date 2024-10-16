/*
    Copyright (C) 2000  Marco Ziech (mmz@gmx.net)
    Copyright (C) 2000  Ross Combs (rocombs@cs.nmsu.edu)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#include "common/setup_before.h"
#include <stdio.h>
#ifdef STDC_HEADERS
# include <stdlib.h>
#else
# ifdef HAVE_MALLOC_H
#  include <malloc.h>
# endif
#endif
#ifdef HAVE_STRING_H
# include <string.h>
#else
# ifdef HAVE_STRINGS_H
#  include <strings.h>
# endif
#endif
#include <errno.h>
#include "compat/strerror.h"
#include "fileio.h"
#include "tga.h"
#include "common/setup_after.h"


static int rotate_updown(t_tgaimg *img);
static int rotate_leftright(t_tgaimg *img);
static int RLE_decompress(FILE *f, void *buf, int bufsize, int pixelsize);
static void RLE_write_pkt(FILE *f, t_tgapkttype pkttype, int len, void *data, int pixelsize);
static int RLE_compress(FILE *f, t_tgaimg const *img);


static int rotate_updown(t_tgaimg *img) {
	unsigned char *ndata;
	int pixelsize;
	int y;
	if (img == NULL) return -1;
	if (img->data == NULL) return -1;
	pixelsize = getpixelsize(img);
	if (pixelsize == 0) return -1;
	ndata = malloc((size_t)img->width * (size_t)img->height * (size_t)pixelsize);
	for (y = 0; y < img->height; y++) {
		memcpy(ndata + ((size_t)y * (size_t)img->width * (size_t)pixelsize),
		       img->data + (((size_t)img->width * (size_t)img->height * (size_t)pixelsize) - ((size_t)(y + 1) * (size_t)img->width * (size_t)pixelsize)),
		       (size_t)img->width * (size_t)pixelsize);
	}
	free(img->data);
	img->data = ndata;
	return 0;
}

static int rotate_leftright(t_tgaimg *img) {
        unsigned char *ndata, *datap;
        int pixelsize;
        int y,x;
	fprintf(stderr,"WARNING: rotate_leftright: this function is untested!\n");
        if (img == NULL) return -1;
        if (img->data == NULL) return -1;
        pixelsize = getpixelsize(img);
        if (pixelsize == 0) return -1;
        ndata = malloc((size_t)img->width * img->height * pixelsize);
	datap = img->data;
        for (y = 0; y < img->height; y++) {
		unsigned char *linep = (ndata + (((size_t)(y + 1) * img->width * pixelsize) - pixelsize));
		for (x = 0; x < img->width; x++) {
			memcpy(linep,datap,pixelsize);
			linep -= pixelsize;
			datap += pixelsize;
		}
        }
        free(img->data);
        img->data = ndata;

	return 0;
}

extern int getpixelsize(t_tgaimg const *img) {
	switch (img->bpp) {
		case 8:
			return 1;
		case 15:
		case 16:
			return 2;
		case 24:
			return 3;
		case 32:
			return 4;
		default:
			fprintf(stderr,"load_tga: color depth %u is not supported!\n",img->bpp);
			return 0;
	}
}


extern t_tgaimg * new_tgaimg(unsigned int width, unsigned int height, unsigned int bpp, t_tgaimgtype imgtype) {
	t_tgaimg *img;

	img = malloc(sizeof(t_tgaimg));
        img->idlen = 0;
        img->cmaptype = tgacmap_none;
        img->imgtype = imgtype;
        img->cmapfirst = 0;
        img->cmaplen = 0;
        img->cmapes = 0;
        img->xorigin = 0;
        img->yorigin = 0;
        img->width = width;
        img->height = height;
        img->bpp = bpp;
        img->desc = 0; /* no attribute bits, top, left, and zero reserved */
	img->data = NULL;
	img->extareaoff = 0;
	img->devareaoff = 0;

	return img;
}

extern t_tgaimg * load_tgaheader(void) {
	t_tgaimg *img;

	img = malloc(sizeof(t_tgaimg));
        img->idlen = file_readb();
        img->cmaptype = file_readb();
        img->imgtype = file_readb();
        img->cmapfirst = file_readw_le();
        img->cmaplen = file_readw_le();
        img->cmapes = file_readb();
        img->xorigin = file_readw_le();
        img->yorigin = file_readw_le();
        img->width = file_readw_le();
        img->height = file_readw_le();
        img->bpp = file_readb();
        img->desc = file_readb();
	img->data = NULL;
	img->extareaoff = 0; /* ignored when reading */
	img->devareaoff = 0; /* ignored when reading */

	return img;
}

extern t_tgaimg * load_tga(FILE *f) {
	t_tgaimg *img;
	int pixelsize;

	file_rpush(f);
	img = load_tgaheader();

	/* make sure we understand the header fields */
	if (img->cmaptype != tgacmap_none) {
		fprintf(stderr,"load_tga: Color-mapped images are not (yet?) supported!\n");
		free(img);
		return NULL;
	}
	if (img->imgtype!=tgaimgtype_uncompressed_truecolor && img->imgtype!=tgaimgtype_rlecompressed_truecolor) {
		fprintf(stderr,"load_tga: imagetype %u is not supported. (only 2 and 10 are supported)\n",img->imgtype);
		free(img);
		return NULL;
	}

	pixelsize = getpixelsize(img);
	if (pixelsize == 0) {
		free(img);
		return NULL;
	}
	/* Skip the ID if there is one */
	if (img->idlen > 0) {
		fprintf(stderr,"load_tga: ID present, skipping %d bytes\n",img->idlen);
		if (fseek(f,img->idlen,SEEK_CUR)<0)
			fprintf(stderr,"load_tga: could not seek %u bytes forward (fseek: %s)\n",img->idlen,strerror(errno));
	}

	/* Now, we can alloc img->data */
	img->data = malloc((size_t)img->width * (size_t)img->height * (size_t)pixelsize);
	if (img->imgtype == tgaimgtype_uncompressed_truecolor) {
		if (fread(img->data, (size_t)pixelsize, (size_t)img->width * (size_t)img->height, f) < (size_t)img->width * (size_t)img->height) {
			fprintf(stderr,"load_tga: error while reading data!\n");
			free(img->data);
			free(img);
			return NULL;
		}
	}
	else { /* == tgaimgtype_rlecompressed_truecolor */
		if (RLE_decompress(f, img->data, (size_t)img->width * (size_t)img->height * (size_t)pixelsize, (size_t)pixelsize) < 0) {
			fprintf(stderr,"load_tga: error while decompressing data!\n");
			free(img->data);
			free(img);
			return NULL;
		}
	}
	file_rpop();
	if ((img->desc & tgadesc_horz) == 1) { /* right, want left */
		if (rotate_leftright(img)<0) {
			fprintf(stderr,"ERROR: rotate_leftright failed!\n");
		}
	}
	if ((img->desc & tgadesc_vert) == 0) { /* bottom, want top */
		if (rotate_updown(img)<0) {
			fprintf(stderr,"ERROR: rotate_updown failed!\n");
		}
	}
	return img;
}

extern int write_tga(FILE *f, t_tgaimg *img) {
	if (f == NULL) return -1;
	if (img == NULL) return -1;
	if (img->data == NULL) return -1;
	if (img->idlen!=0) return -1;
	if (img->cmaptype!=tgacmap_none) return -1;
	if (img->imgtype!=tgaimgtype_uncompressed_truecolor && img->imgtype!=tgaimgtype_rlecompressed_truecolor) return -1;
	file_wpush(f);

	file_writeb(img->idlen);
	file_writeb(img->cmaptype);
	file_writeb(img->imgtype);
	file_writew_le(img->cmapfirst);
	file_writew_le(img->cmaplen);
	file_writeb(img->cmapes);
	file_writew_le(img->xorigin);
	file_writew_le(img->yorigin);
	file_writew_le(img->width);
	file_writew_le(img->height);
	file_writeb(img->bpp);
	file_writeb(img->desc);

	if ((img->desc & tgadesc_horz) == 1) { /* right, want left */
	        fprintf(stderr,"write_tga: flipping horizontally\n");
		if (rotate_leftright(img)<0) {
			fprintf(stderr,"ERROR: rotate_updown failed!\n");
		}
	}
	if ((img->desc&tgadesc_vert)==0) { /* bottom, want top */
	        fprintf(stderr,"write_tga: flipping vertically\n");
		if (rotate_updown(img)<0) {
			fprintf(stderr,"ERROR: rotate_updown failed!\n");
		}
	}
	if (img->imgtype==tgaimgtype_uncompressed_truecolor) {
		int pixelsize;

		pixelsize = getpixelsize(img);
		if (pixelsize == 0) return -1;
		if (fwrite(img->data, pixelsize, (size_t)img->width * img->height, f) < (size_t)img->width * img->height) {
			fprintf(stderr,"write_tga: could not write %zu pixels (fwrite: %s)\n", (size_t)img->width * img->height, strerror(errno));
			file_wpop();
			return -1;
		}
	} else if (img->imgtype==tgaimgtype_rlecompressed_truecolor) {
	        fprintf(stderr,"write_tga: using RLE compression\n");
		if (RLE_compress(f,img)<0) {
			fprintf(stderr,"write_tga: RLE compression failed.\n");
		}
	}
	/* Write the file-footer */
	file_writed_le(img->extareaoff);
	file_writed_le(img->devareaoff);
	if (fwrite(TGAMAGIC,strlen(TGAMAGIC)+1,1,f)<1)
		fprintf(stderr,"write_tga: could not write TGA footer magic (fwrite: %s)\n",strerror(errno));
	/* Ready */
	file_wpop();
	return 0;
}

static int RLE_decompress(FILE *f, void *buf, int bufsize, int pixelsize) {
	unsigned char pt;
	unsigned char *bufp;
	unsigned char temp[8]; /* MAXPIXELSIZE */
	int bufi;
	int count;

	file_rpush(f);
	bufp = buf;
	for (bufi=0; bufi<bufsize; ) {
		pt = file_readb();
		if (feof(f)) {
			fprintf(stderr,"RLE_decompress: after final packet only got %d of %d bytes\n",bufi,bufsize);
			file_rpop();
			return -1;
		}
		count = (pt & 0x7f)+1;
		if (bufi+count*pixelsize>bufsize) {
			fprintf(stderr,"RLE_decompress: buffer too short for next packet (need %d bytes, have %d)\n",bufi+count*pixelsize,bufsize);
			file_rpop();
			return -1;
		}
		if ((pt & 0x80) == 0) {	/* RAW PACKET */
			if (fread(bufp,pixelsize,count,f)<count) {
				if (feof(f))
					fprintf(stderr,"RLE_decompress: short RAW packet (expected %d bytes) (EOF)\n",pixelsize*count);
				else
					fprintf(stderr,"RLE_decompress: short RAW packet (expected %d bytes) (fread: %s)\n",pixelsize*count,strerror(errno));
#if 0
				file_rpop();
				return -1;
#endif
			}
			bufp += count*pixelsize;
			bufi += count*pixelsize;
		} else { /* RLE PACKET */
			if (fread(temp,pixelsize,1,f) < 1) {
				if (feof(f))
					fprintf(stderr,"RLE_decompress: short RLE packet (expected %d bytes) (EOF)\n",pixelsize);
				else
					fprintf(stderr,"RLE_decompress: short RLE packet (expected %d bytes) (fread: %s)\n",pixelsize,strerror(errno));
#if 0
				file_rpop();
				return -1;
#endif
			}
			if (count<2) {
				fprintf(stderr,"RLE_decompress: suspicious RLE repetition count %d\n",count);
			}
			for (;count > 0; count--) {
				memcpy(bufp,temp,pixelsize);
				bufp += pixelsize;
				bufi += pixelsize;
			}
		}
	}
	file_rpop();
	return 0;
}

static void RLE_write_pkt(FILE *f, t_tgapkttype pkttype, int len, void *data, int pixelsize) {
	unsigned char count;

	if (len<1 || len>128) {
		fprintf(stderr,"RLE_write_pkt: packet has bad length (%d bytes)\n",len);
		return;
	}
	if (pkttype==RLE) {
		if (len<2) {
			fprintf(stderr,"RLE_write_pkt: RLE packet has bad length (%d bytes)\n",len);
			return;
		}
		count = (unsigned char)(0x80 | (len-1));
		if (fwrite(&count, 1, 1, f)<1)
			fprintf(stderr,"RLE_write_pkt: could not write RLE pixel count (fwrite: %s)\n",strerror(errno));
		if (fwrite(data, pixelsize, 1, f)<1)
			fprintf(stderr,"RLE_write_pkt: could not write RLE pixel value (fwrite: %s)\n",strerror(errno));
	} else {
		count = (unsigned char) (len-1);
		if (fwrite(&count, 1, 1, f)<1)
			fprintf(stderr,"RLE_write_pkt: could not write RAW pixel count (fwrite: %s)\n",strerror(errno));
		if (fwrite(data,pixelsize,len,f)<len)
			fprintf(stderr,"RLE_write_pkt: could not write %d RAW pixels (fwrite: %s)\n",len,strerror(errno));
	}
}


static int RLE_compress(FILE *f, t_tgaimg const *img) {
	int pixelsize;
	unsigned char const *datap;
	unsigned char *pktdata;
	unsigned int pktlen;
	t_tgapkttype pkttype;
	unsigned char *pktdatap;
	unsigned int actual=0,perceived=0;
	int i;

	if (img == NULL) return -1;
	if (img->data == NULL) return -1;
	pixelsize = getpixelsize(img);
	if (pixelsize == 0) return -1;

	datap = img->data;
	pktdata = malloc((size_t)img->width * (size_t)img->height * (size_t)pixelsize);
	pktlen = 0;

	for (i=0; i<img->width*img->height; ) {
		if (pktlen == 0) {
			pktdatap = pktdata;
			memcpy(pktdatap,datap,pixelsize);
			pktlen++;
			i++;
			pktdatap += pixelsize;
			datap += pixelsize;
			pkttype = RAW;
			continue;
		}
		if (pktlen == 1) {
			if (memcmp(datap-pixelsize,datap,pixelsize)==0) {
				pkttype = RLE;
			}
		}
		if (pkttype == RLE) {
			if (memcmp(datap-pixelsize,datap,pixelsize)!=0 || pktlen>=128) {
				RLE_write_pkt(f,pkttype,pktlen,pktdata,pixelsize);
				actual += 1+pixelsize;
				perceived += pixelsize*pktlen;
				pktlen = 0;
			} else {
				pktlen++;
				i++;
				datap += pixelsize;
			}
		} else {
			if (memcmp(datap-pixelsize,datap,pixelsize)==0 || pktlen>=129) {
				datap -= pixelsize; /* push back last pixel */
				i--;
				if (i<0) fprintf(stderr,"BUG!\n");
				pktlen--;
				RLE_write_pkt(f,pkttype,pktlen,pktdata,pixelsize);
				actual += 1+pixelsize*pktlen;
				perceived += pixelsize*pktlen;
				pktlen = 0;
			} else {
				memcpy(pktdatap,datap,pixelsize);
				pktlen++;
				i++;
				pktdatap += pixelsize;
				datap += pixelsize;
			}
		}
	}
	if (pktlen) {
		RLE_write_pkt(f,pkttype,pktlen,pktdata,pixelsize);
		if (pkttype==RLE) {
			actual += 1+pixelsize;
			perceived += pixelsize*pktlen;
		} else {
			actual += 1+pixelsize*pktlen;
			perceived += pixelsize*pktlen;
		}
		pktlen = 0;
	}
	fprintf(stderr,"RLE_compress: wrote %u bytes (%u uncompressed)\n",actual,perceived);
	return 0;
}

extern void destroy_img(t_tgaimg * img) {
	if (img == NULL) return;

	if (img->data)
		free(img->data);
	free(img);
}

extern void print_tga_info(t_tgaimg const * img, FILE * fp) {
	unsigned int interleave;
	unsigned int attrbits;
 	char const * typestr;
 	char const * cmapstr;
	char const * horzstr;
	char const * vertstr;
	char const * intlstr;

	if (!img || !fp)
		return;

	interleave = ((img->desc&tgadesc_interleave1)!=0)*2+((img->desc&tgadesc_interleave2)!=0);
	attrbits = img->desc&(tgadesc_attrbits0|tgadesc_attrbits1|tgadesc_attrbits2|tgadesc_attrbits3);
	switch (img->imgtype) {
	case tgaimgtype_empty:
		typestr = "No Image Data Included";
		break;
	case tgaimgtype_uncompressed_mapped:
		typestr = "Uncompressed, Color-mapped Image";
		break;
	case tgaimgtype_uncompressed_truecolor:
		typestr = "Uncompressed, True-color Image";
		break;
	case tgaimgtype_uncompressed_monochrome:
		typestr = "Uncompressed, Black-and-white image";
		break;
	case tgaimgtype_rlecompressed_mapped:
		typestr = "Run-length encoded, Color-mapped Image";
		break;
	case tgaimgtype_rlecompressed_truecolor:
		typestr = "Run-length encoded, True-color Image";
		break;
	case tgaimgtype_rlecompressed_monochrome:
		typestr = "Run-length encoded, Black-and-white image";
		break;
	case tgaimgtype_huffman_mapped:
		typestr = "Huffman encoded, Color-mapped image";
		break;
        case tgaimgtype_huffman_4pass_mapped:
		typestr = "Four-pass Huffman encoded, Color-mapped image";
		break;
	default:
		typestr = "unknown";
	}
	switch (img->cmaptype) {
	case tgacmap_none:
		cmapstr = "None";
		break;
	case tgacmap_included:
		cmapstr = "Included";
		break;
	default:
		cmapstr = "Unknown";
	}
	if ((img->desc&tgadesc_horz)==0) {
		horzstr = "left";
	} else {
		horzstr = "right";
	}
	if ((img->desc&tgadesc_vert)==0) {
		vertstr = "bottom";
	} else {
		vertstr = "top";
	}
	switch (interleave) {
	case 0:
		intlstr = "none";
		break;
	case 2:
		intlstr = "two way";
		break;
	case 3:
		intlstr = "four way";
		break;
	case 4:
	default:
		intlstr = "unknown";
		break;
	}

	fprintf(fp,"TGAHeader: IDLength=%u ColorMapType=%u(%s)\n",img->idlen,img->cmaptype,cmapstr);
	fprintf(fp,"TGAHeader: ImageType=%u(%s)\n",img->imgtype,typestr);
	fprintf(fp,"TGAHeader: ColorMap: FirstEntryIndex=%u ColorMapLength=%u\n",img->cmapfirst,img->cmaplen);
	fprintf(fp,"TGAHeader: ColorMap: ColorMapEntrySize=%ubits\n",img->cmapes);
	fprintf(fp,"TGAHeader: X-origin=%u Y-origin=%u Width=%u(0x%x) Height=%u(0x%x)\n",img->xorigin,img->yorigin,img->width,img->width,img->height,img->height);
	fprintf(fp,"TGAHeader: PixelDepth=%ubits ImageDescriptor=0x%02x(%u attribute bits, origin is %s %s, interleave=%s)\n",img->bpp,img->desc,attrbits,vertstr,horzstr,intlstr);
}
