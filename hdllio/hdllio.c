/*
 *	hdllio.c	(HD Low-Level I/O)
 *
 *	Copyright (c) 2003, Jiann-Ching Liu
 */

#ifndef _LARGEFILE_SOURCE
#define _LARGEFILE_SOURCE
#endif

#ifndef _LARGEFILE64_SOURCE
#define _LARGEFILE64_SOURCE
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <sys/utsname.h>


// #define HAVE_EXT2FS_LLSEEK

// #ifdef HAVE_EXT2FS_LLSEEK

#include <ext2fs/ext2_fs.h>
#include <ext2fs/ext2fs.h>
// #include <ext2fs/ext2_io.h>

//#endif

#include <linux/unistd.h>
#include <linux/major.h>	/* FLOPPY_MAJOR */
#include <linux/hdreg.h>	/* for HDIO_GETGEO */

#include "hdllio.h"

struct systypes_t {
	unsigned char	type;
	char		*name;
};

#include "include/i386_sys_types.c"


#define DEFAULT_SECTOR_SIZE	512

#define BLKRRPART  _IO(0x12,95)    /* re-read partition table */
#define BLKGETSIZE _IO(0x12,96)    /* return device size */
#define BLKFLSBUF  _IO(0x12,97)    /* flush buffer cache */
#define BLKSSZGET  _IO(0x12,104)   /* get block device sector size */

#define MAKE_VERSION(p,q,r)     (65536*(p) + 256*(q) + (r))

#define pt_offset(b, n) ((struct partition *)((b) + 0x1be + \
					(n) * sizeof(struct partition)))

#define mbr_sector(s)		((s) & 0x3f)
#define mbr_cylinder(s,c)	((c) | (((s) & 0xc0) << 2))

#define ACTIVE_FLAG	0x80
#define EXTENDED	0x05
#define WIN98_EXTENDED	0x0f
#define LINUX_PARTITION	0x81
#define LINUX_SWAP	0x82
#define LINUX_NATIVE	0x83
#define LINUX_EXTENDED	0x85
#define LINUX_LVM	0x8e
#define LINUX_RAID	0xfd

#define IS_EXTENDED(i) \
	((i) == EXTENDED || (i) == WIN98_EXTENDED || (i) == LINUX_EXTENDED)

#define cround(n)	(pd->display_in_cyl_units ? \
				((n)/pd->units_per_sector)+1 : (n))



static int linux_version_code (void) {
	static int	kernel_version = 0;
	struct utsname	my_utsname;
	int		p, q, r;

	if (! kernel_version && uname (&my_utsname) == 0) {
		p = atoi (strtok (my_utsname.release, "."));
		q = atoi (strtok (NULL, "."));
		r = atoi (strtok (NULL, "."));

		kernel_version = MAKE_VERSION(p,q,r);
	}

	return kernel_version;
}

static char *partnamebf (char *dev, int pno, int lth, int bufsiz, char *bufp) {
	static char	buffer[80];
	char		*p;
	int		w, wp;

	if (!bufp) {
		bufp = buffer;
		bufsiz = sizeof(buffer);
	}

	w = strlen(dev);
	p = ""; 

	if (isdigit (dev[w-1])) p = "p";

	if (strcmp (dev + w - 4, "disc") == 0) {
		w -= 4;
		p = "part";
	}

	wp = strlen(p);

	if (lth) {
		sprintf (bufp, "%*.*s%s%-2u", lth-wp-2, w, dev, p, pno);
	} else {
		sprintf (bufp, "%.*s%s%-2u", w, dev, p, pno);
	}

	return bufp;
}

static char * partname (char *dev, int pno, int lth) {
	return partnamebf(dev, pno, lth, 0, NULL);
}

static char *partition_type (unsigned char type) {
	int			i;
	struct systypes_t	*types = i386_sys_types;

	for (i = 0; types[i].name; i++) {
		if (types[i].type == type) return types[i].name;
	}

	return NULL;
}


/* start_sect and nr_sects are stored little endian on all machines */
/* moreover, they are not aligned correctly */

static void store4_little_endian (unsigned char *cp, unsigned int val) {
	cp[0] = (val & 0xff);
	cp[1] = ((val >> 8) & 0xff);
	cp[2] = ((val >> 16) & 0xff);
	cp[3] = ((val >> 24) & 0xff);
}

static unsigned int read4_little_endian (unsigned char *cp) {
	return (unsigned int)(cp[0]) + ((unsigned int)(cp[1]) << 8)
			+ ((unsigned int)(cp[2]) << 16) +
			((unsigned int)(cp[3]) << 24);
}

static void set_start_sect (struct partition *p, unsigned int start_sect) {
	store4_little_endian (p->start4, start_sect);
}

unsigned int get_start_sect (struct partition *p) {
	return read4_little_endian (p->start4);
}

static void set_nr_sects (struct partition *p, unsigned int nr_sects) {
	store4_little_endian (p->size4, nr_sects);
}

unsigned int get_nr_sects(struct partition *p) {
	return read4_little_endian (p->size4);
}

//

#ifndef HAVE_EXT2FS_LLSEEK

static int _llseek (unsigned int fd,  unsigned  long  offset_high,
		unsigned  long  offset_low,  loff_t  *result, unsigned int
		whence);
#ifdef __NR__llseek


static _syscall5 (int,_llseek,unsigned int,fd,unsigned long,offset_high,
		unsigned long, offset_low,ext2_loff_t *,result,
		unsigned int, origin)

#else
/* no __NR__llseek on compilation machine - might give it explicitly */
static int _llseek (unsigned int fd, unsigned long oh,
		unsigned long ol, ext2_loff_t *result,
		unsigned int origin) {
	errno = ENOSYS;
	return -1;
}
#endif
#endif

ext2_loff_t my_llseek (int fd, ext2_loff_t offset, int origin) {
#ifdef HAVE_EXT2FS_LLSEEK
	return ext2fs_llseek (fd, offset, origin);
#else

	ext2_loff_t		result;
	int			retval;

	retval = _llseek (fd, ((unsigned long long) offset) >> 32,
				((unsigned long long) offset) & 0xffffffff,
				&result, origin);

	return (retval == -1 ? (ext2_loff_t) retval : result);
#endif
}

static void seek_sector (struct hd_lowlevel_io_t *self, unsigned int secno) {
	struct hd_lowlevel_io_pd_t	*pd = &self->pd;

	ext2_loff_t offset = (ext2_loff_t) secno * pd->sector_size;

	if (my_llseek (pd->fd, offset, SEEK_SET) == (ext2_loff_t) -1) {
		perror ("seek_sector");
	}
}

static unsigned int get_partition_start (struct pte *pe) {
	return pe->offset + get_start_sect (pe->part_table);
}

static int read_sector (struct hd_lowlevel_io_t *self,
						unsigned int secno, char *buf) {
	struct hd_lowlevel_io_pd_t	*pd = &self->pd;

	seek_sector (self, secno);

	if (read (pd->fd, buf, pd->sector_size) != pd->sector_size) {
		perror ("read_sector");
		return 0;
	}

	return 1;
}

static int write_sector (struct hd_lowlevel_io_t *self,
						unsigned int secno, char *buf) {
	struct hd_lowlevel_io_pd_t	*pd = &self->pd;

	seek_sector (self, secno);

	if (write (pd->fd, buf, pd->sector_size) != pd->sector_size) {
		perror ("write_sector");
		return 0;
	}

	return 1;
}

static int read_sectors (struct hd_lowlevel_io_t *self,
					unsigned int start_sec,
					unsigned int num_sec,
					char *buffer) {
	struct hd_lowlevel_io_pd_t	*pd = &self->pd;
	int				expsize;

	seek_sector (self, start_sec);

	expsize = num_sec * pd->sector_size;

	if (read (pd->fd, buffer, expsize) != expsize) {
		perror ("read_sectors");
		return 0;
	}

	return 1;
}

static int write_sectors (struct hd_lowlevel_io_t *self,
					unsigned int start_sec,
					unsigned int num_sec,
					char *buffer) {
	struct hd_lowlevel_io_pd_t	*pd = &self->pd;
	int				expsize;

	seek_sector (self, start_sec);

	expsize = num_sec * pd->sector_size;

	if (write (pd->fd, buffer, expsize) != expsize) {
		perror ("write_sectors");
		return 0;
	}

	return 1;
}

static int is_cleared_partition (struct partition *p) {
	return !(!p || p->boot_ind || p->head || p->sector || p->cyl ||
		p->sys_ind || p->end_head || p->end_sector || p->end_cyl ||
		get_start_sect(p) || get_nr_sects(p));
}

static void guess_device_type (struct hd_lowlevel_io_t *self) {
	struct hd_lowlevel_io_pd_t	*pd = &self->pd;
	struct stat			bootstat;
	int				fd = self->pd.fd;

	if (fstat (fd, &bootstat) < 0) {
		pd->scsi_disk = 0;
		pd->floppy = 0;
	} else if (S_ISBLK(bootstat.st_mode)
			&& ((bootstat.st_rdev >> 8) == IDE0_MAJOR ||
			(bootstat.st_rdev >> 8) == IDE1_MAJOR)) {
		pd->scsi_disk = 0;
		pd->floppy = 0;
	} else if (S_ISBLK(bootstat.st_mode)
			&& (bootstat.st_rdev >> 8) == FLOPPY_MAJOR) {
		pd->scsi_disk = 0;
		pd->floppy = 1;
	} else {
		pd->scsi_disk = 1;
		pd->floppy = 0;
	}
}

static void get_sectorsize (struct hd_lowlevel_io_t *self) {
#if defined(BLKSSZGET)
	if (! self->pd.user_set_sector_size &&
			linux_version_code() >= MAKE_VERSION(2,3,3)) {
		int	arg;
		int	fd = self->pd.fd;

		if (ioctl (fd, BLKSSZGET, &arg) == 0) {
			self->pd.sector_size = arg;
		}

		if (self->pd.sector_size != DEFAULT_SECTOR_SIZE) {
			printf("Note: sector size is %d (not %d)\n",
					self->pd.sector_size,
					DEFAULT_SECTOR_SIZE);
		}
	}
#else
	/* maybe the user specified it; and otherwise we still
	 *            have the DEFAULT_SECTOR_SIZE default */
#endif
}

void update_units (struct hd_lowlevel_io_t *self) {
	struct hd_lowlevel_io_pd_t	*pd = &self->pd;
	int				cyl_units = pd->heads * pd->sectors;

	if (pd->display_in_cyl_units && cyl_units) {
		pd->units_per_sector = cyl_units;
	} else {
		pd->units_per_sector = 1;   /* in sectors */
	}
}

static void internal_get_geometry (struct hd_lowlevel_io_t *self) {
	struct hd_lowlevel_io_pd_t	*pd = &self->pd;
	int				sec_fac;
	long				longsectors;
	struct hd_big_geometry		geometry;
	struct hd_geometry		smallgeo;
	int				res1, res2,res3;
	int				fd;

	fd = pd->fd;

	self->get_sectorsize (self);
	sec_fac = pd->sector_size / 512;
	self->guess_device_type (self);

	res1 = ioctl (fd, BLKGETSIZE, &longsectors);
	res2 = ioctl (fd, HDIO_GETGEO_BIG, &geometry);
	res3 = ioctl (fd, HDIO_GETGEO, &smallgeo);

	pd->heads = pd->cylinders = pd->sectors = 0;
	pd->sector_offset = 1;

	if (res2 == 0) {
		pd->heads = geometry.heads;
		pd->sectors = geometry.sectors;

		if (pd->heads * pd->sectors == 0) {
			res2 = -1;
		} else if (pd->dos_compatible_flag) {
			pd->sector_offset = pd->sectors;
		}
	} else if (res3 == 0) {
		pd->heads = smallgeo.heads;
		pd->sectors = smallgeo.sectors;

		if (pd->heads * pd->sectors == 0) {
			res2 = -1;
		} else if (pd->dos_compatible_flag) {
			pd->sector_offset = pd->sectors;
		}
	}

	if (res1 == 0 && (res2 == 0 || res3 == 0)) {   /* normal case */
		pd->cylinders = longsectors / (pd->heads * pd->sectors);
		pd->cylinders /= sec_fac;   /* do not round up */
	} else if (res1 == 0) {         /* size but no geometry */
		pd->heads = pd->cylinders = 1;
		pd->sectors = longsectors / sec_fac;
	}

	if (! pd->sectors) pd->sectors = pd->user_sectors;
	if (! pd->heads) pd->heads = pd->user_heads;
	if (! pd->cylinders) pd->cylinders = pd->user_cylinders;

	/*
	if (g) {
		g->heads     = pd->heads;
		g->sectors   = pd->sectors;
		g->cylinders = pd->cylinders;
	}
	*/
}

static void get_geometry (struct hd_lowlevel_io_t *self,
					struct hd_geometry_t *g) {
	if (g) {
		g->heads     = self->pd.heads;
		g->sectors   = self->pd.sectors;
		g->cylinders = self->pd.cylinders;
	}
}

static void clear_partition (struct partition *p) {
	if (! p) return;

	p->boot_ind = 0;
	p->head = 0;
	p->sector = 0;
	p->cyl = 0;
	p->sys_ind = 0;
	p->end_head = 0;
	p->end_sector = 0;
	p->end_cyl = 0;

	set_start_sect (p, 0);
	set_nr_sects (p, 0);
}

static void read_extended (struct hd_lowlevel_io_t *self, int ext) {
	struct hd_lowlevel_io_pd_t	*pd = &self->pd;
	int				i;
	struct pte			*pex;
	struct partition		*p, *q;

	pd->ext_index = ext;
	pex = &pd->ptes[ext];
	pex->ext_pointer = pex->part_table;

	p = pex->part_table;

	if (! get_start_sect (p)) {
		fprintf (stderr, "Bad offset in primary extended partition\n");
		return;
	}

	while (IS_EXTENDED (p->sys_ind)) {
		struct pte	*pe = &pd->ptes[pd->partitions];

		if (pd->partitions >= MAXIMUM_PARTS) {
			/* This is not a Linux restriction, but
			   this program uses arrays of size MAXIMUM_PARTS.
			   Do not try to `improve' this test. */
			struct pte	*pre = &pd->ptes[pd->partitions - 1];

			fprintf (stderr,
				"Warning: deleting partitions after %d\n",
				pd->partitions);

			clear_partition (pre->ext_pointer);
			// pre->changed = 1;
			return;
		}

		pe->start_sector = pd->extended_offset + get_start_sect (p);

		self->read_pte (self, pd->partitions, pe->start_sector);

		if (! pd->extended_offset) {
			pd->extended_offset = get_start_sect (p);
		}

		q = p = pt_offset (pe->sectorbuffer, 0);


		for (i = 0; i < 4; i++, p++) if (get_nr_sects(p)) {
			if (IS_EXTENDED (p->sys_ind)) {
				if (pe->ext_pointer) {
					fprintf (stderr,
						"Warning: extra link "
						"pointer in partition table"
						" %d\n", pd->partitions + 1);
				} else {
					pe->ext_pointer = p;
				}
			} else if (p->sys_ind) {
				if (pe->part_table) {
					fprintf (stderr,
						"Warning: ignoring extra "
						"data in partition table"
						" %d\n", pd->partitions + 1);
				} else {
					pe->part_table = p;
					pe->start_sector += get_start_sect (p);
					pe->num_of_sectors = get_nr_sects (p);

					/*
					fprintf (stderr,
					    "ext %d. [%02x] C=%u,H=%u,S=%u\n",
					    i + 1,
						pe->part_table->sys_ind,
						mbr_cylinder (
							pe->part_table->sector,
							pe->part_table->cyl),
						pe->part_table->head,
						mbr_sector (
						   pe->part_table->sector));
					*/
				}
			}
		}

		/* very strange code here... */

		if (! pe->part_table) {
			if (q != pe->ext_pointer) {
				pe->part_table = q;
			} else {
				pe->part_table = q + 1;
			}
		}

		if (! pe->ext_pointer) {
			if (q != pe->part_table) {
				pe->ext_pointer = q;
			} else {
				pe->ext_pointer = q + 1;
			}
		}

		p = pe->ext_pointer;
		pd->partitions++;
	}

	for (i = 4; i < pd->partitions; i++) {
		struct pte	*pe = &pd->ptes[i];

		if (!get_nr_sects(pe->part_table) &&
				(pd->partitions > 5 ||
				 pd->ptes[4].part_table->sys_ind)) {
			fprintf (stderr,
					"omitting empty partition (%d)\n", i+1);
			// delete_partition(i);
			// goto remove;    /* numbering changed */
		}
	}
}

static void read_pte (struct hd_lowlevel_io_t *self,
				int pno, unsigned int offset) {
	struct hd_lowlevel_io_pd_t	*pd = &self->pd;
	struct pte			*pe = &pd->ptes[pno];

	// fprintf (stderr, "READ PTE [%u]\n", offset);

	pe->offset = offset;

	pe->sectorbuffer = (char *) malloc (pd->sector_size);

	if (!pe->sectorbuffer) {
		perror ("read pte");
		return;
	}

	self->read_sector (self, offset, pe->sectorbuffer);
	// pe->changed = 0;
	pe->part_table = pe->ext_pointer = NULL;
}

static int hdll_open (struct hd_lowlevel_io_t *self, const char *device) {
	struct hd_lowlevel_io_pd_t	*pd = &self->pd;
	int				i;

	strcpy (pd->disk_device, device);

	self->close (self);

	if ((pd->fd = open (device, O_RDONLY|O_LARGEFILE)) < 0) {
		perror (device);
		return 0;
	}

	internal_get_geometry (self);
	update_units (self);

	/*
	fprintf (stderr, "C=%u,H=%u,S=%u\n",
			pd->cylinders,
			pd->heads,
			pd->sectors);
	*/

	if (pd->sector_size != read (pd->fd, pd->MBRbuffer, pd->sector_size)) {
		perror ("read_MBR");
		self->close (self);
		return 0;
	}

	for (i = 0; i < 4; i++) {
		struct pte	*pe = &pd->ptes[i];

		pe->part_table = pt_offset (pd->MBRbuffer, i);
		pe->ext_pointer = NULL;
		pe->offset = 0;
		pe->sectorbuffer = pd->MBRbuffer;
		// pe->changed = (what == create_empty_dos);

		pe->start_sector = get_start_sect (pe->part_table);
		pe->num_of_sectors = get_nr_sects (pe->part_table);

		/*
		fprintf (stderr,
			"%d. [%02x] C=%u,H=%u,S=%u\n",
			i + 1,
			pe->part_table->sys_ind,
			mbr_cylinder (pe->part_table->sector,
						pe->part_table->cyl),
			pe->part_table->head,
			mbr_sector (pe->part_table->sector));
		*/
	}

	for (i = 0; i < 4; i++) {
		struct pte	*pe = &pd->ptes[i];

		if (IS_EXTENDED (pe->part_table->sys_ind)) {
			if (pd->partitions != 4) {
				fprintf(stderr, "Ignoring extra extended "
					"partition %d\n", i + 1);
			} else {
				self->read_extended (self, i);
			}
		}
	}

	return 1;
}

static void hdll_close (struct hd_lowlevel_io_t *self) {
	if (self->pd.fd >= 0) {
		close (self->pd.fd);
		self->pd.fd = -1;
	}
}

static void hdll_list_table (struct hd_lowlevel_io_t *self) {
	struct hd_lowlevel_io_pd_t	*pd = &self->pd;
	struct partition		*p;
	// char				*type;
	int				i, w;
	// int				xtra = 0;

	w = strlen (pd->disk_device);
	if (w && isdigit(pd->disk_device[w-1])) w++;
	if (w < 5) w = 5;

	printf ("%*s Boot    Start       End    Blocks   Id  System\n",
			w + 1, pd->disk_device);

	for (i = 0 ; i < pd->partitions; i++) {
		struct pte	*pe = &pd->ptes[i];

		p = pe->part_table;

		if (p && !is_cleared_partition(p)) {
			unsigned int	psects = get_nr_sects (p);
			unsigned int	pblocks = psects;
			unsigned int	podd = 0;

			if (pd->sector_size < 1024) {
				pblocks /= (1024 / pd->sector_size);
				podd = psects % (1024 / pd->sector_size);
			}

			if (pd->sector_size > 1024) {
				pblocks *= (pd->sector_size / 1024);
			}

			printf ("%s  %c %9ld %9ld %9ld%c  %2x  %s\n",
				partname (pd->disk_device, i+1, w+2),
/* boot flag */			!p->boot_ind ? ' ' : p->boot_ind == ACTIVE_FLAG
					? '*' : '?',
/* start */			(long) cround(get_partition_start(pe)),
/* end */			(long) cround(get_partition_start(pe) + psects
					- (psects ? 1 : 0)),
/* odd flag on end */		(long) pblocks, podd ? '+' : ' ',
/* type id */			p->sys_ind,
				partition_type (p->sys_ind));
			// check_consistency(p, i);
		}
	}
}

static void hdll_xlist_table (struct hd_lowlevel_io_t *self) {
	struct hd_lowlevel_io_pd_t	*pd = &self->pd;
	int				i;
	struct pte			*pe;
	struct partition		*p;
	int				extend = 0;

	printf ("\nDisk: %d heads, %d sectors, %d cylinders\n\n",
		pd->heads, pd->sectors, pd->cylinders);

	printf("Nr AF  Hd Sec  Cyl  Hd Sec  Cyl    Start     Size ID\n");

	for (i = 0 ; i < pd->partitions; i++) {
		pe = &pd->ptes[i];

		p = (extend ? pe->ext_pointer : pe->part_table);

		if (p != NULL) {
			printf ("%2d %02x%4d%4d%5d%4d%4d%5d%9d%9d %02x\n",
				i + 1, p->boot_ind, p->head,
				mbr_sector (p->sector),
				mbr_cylinder (p->sector, p->cyl),
				p->end_head,
				mbr_sector (p->end_sector),
				mbr_cylinder (p->end_sector, p->end_cyl),
				get_start_sect(p), get_nr_sects(p), p->sys_ind);

			// if (p->sys_ind) check_consistency(p, i);
		}
	}
}

static int number_of_partitions (struct hd_lowlevel_io_t *self) {
	return self->pd.partitions;
}

static struct pte * get_partition_entry (
				struct hd_lowlevel_io_t *self, const int n) {
	return &self->pd.ptes[n];
}

static int sector_size (struct hd_lowlevel_io_t *self) {
	return self->pd.sector_size;
}

static int cylinder_size (struct hd_lowlevel_io_t *self) {
	return self->pd.units_per_sector;
}

static char * partition_sysname (struct hd_lowlevel_io_t *self, const int n) {
	return partition_type (self->pd.ptes[n].part_table->sys_ind);
}

struct hd_lowlevel_io_t	*new_hd_lowlevel_io (void) {
	struct hd_lowlevel_io_t		*self;
	struct hd_lowlevel_io_pd_t	*pd;

	if ((self = malloc (sizeof (struct hd_lowlevel_io_t))) != NULL) {
		pd	= &self->pd;

		pd->fd			= -1;
		pd->heads		= 0;
		pd->sectors		= 0;
		pd->cylinders		= 0;
		pd->sector_size		= DEFAULT_SECTOR_SIZE;
		pd->scsi_disk		= 0;
		pd->floppy		= 0;
		pd->sector_offset	= 1;
		pd->partitions		= 4;
		pd->ext_index		= 0;
		pd->extended_offset	= 0;
		pd->display_in_cyl_units= 1;
		pd->units_per_sector	= 1;

		pd->user_set_sector_size	= 0;
		pd->dos_compatible_flag	= ~0;

		self->open		= hdll_open;
		self->close		= hdll_close;
		self->get_sectorsize	= get_sectorsize;
		self->get_geometry	= get_geometry;
		self->guess_device_type	= guess_device_type;
		self->read_pte		= read_pte;
		self->read_extended	= read_extended;
		self->list_table	= hdll_list_table;
		self->xlist_table	= hdll_xlist_table;
		self->read_sector	= read_sector;
		self->write_sector	= write_sector;
		self->read_sectors	= read_sectors;
		self->write_sectors	= write_sectors;
		self->num_of_partitions	= number_of_partitions;
		self->get_partition_entry	= get_partition_entry;
		self->sector_size	= sector_size;
		self->cylinder_size	= cylinder_size;
		self->partition_sysname	= partition_sysname;
	}

	return self;
}
