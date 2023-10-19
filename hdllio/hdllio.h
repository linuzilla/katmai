/*
 *	hdllio.h	(HD Low-Level I/O)
 *
 *	Copyright (c) 2003, Jiann-Ching Liu
 */

#ifndef __HDLLIO_H__
#define __HDLLIO_H__

#define MAX_SECTOR_SIZE		2048
#define MAXIMUM_PARTS		16

struct partition {
	unsigned char	boot_ind;	/* 0x80 - active */
	unsigned char	head;		/* starting head */
	unsigned char	sector;		/* starting sector */
	unsigned char	cyl;		/* starting cylinder */
	unsigned char	sys_ind;	/* What partition type */
	unsigned char	end_head;	/* end head */
	unsigned char	end_sector;	/* end sector */
	unsigned char	end_cyl;	/* end cylinder */
	unsigned char	start4[4];	/* starting sector counting from 0 */
	unsigned char	size4[4];	/* nr of sectors in partition */
};

struct hd_geometry_t {
	unsigned int	heads;
	unsigned int	sectors;
	unsigned int	cylinders;
};

struct pte {
	struct partition	*part_table;   /* points into sectorbuffer */
	struct partition	*ext_pointer;  /* points into sectorbuffer */
	char			changed;           /* boolean */
	unsigned int		offset;            /* disk sector number */
	char			*sectorbuffer;     /* disk sector contents */

	unsigned int		start_sector;
	unsigned int		num_of_sectors;
};

struct hd_lowlevel_io_pd_t {
	int		fd;
	unsigned int	heads;
	unsigned int	sectors;
	unsigned int	cylinders;
	unsigned int	sector_size;
	unsigned int	sector_offset;

	unsigned int	user_set_sector_size;
	unsigned int	user_cylinders;
	unsigned int	user_heads;
	unsigned int	user_sectors;

	int		dos_compatible_flag;
	int		partitions;
	int		ext_index;
	unsigned int	extended_offset;

	int		scsi_disk;
	int		floppy;

	char		MBRbuffer[MAX_SECTOR_SIZE];
	struct pte	ptes[MAXIMUM_PARTS];

	char		disk_device[16];
	unsigned int	display_in_cyl_units;
	unsigned int	units_per_sector;
};

struct hd_lowlevel_io_t {
	struct hd_lowlevel_io_pd_t	pd;

	int		(*open)(struct hd_lowlevel_io_t *self,
						const char *device);
	void		(*close)(struct hd_lowlevel_io_t *self);
	void		(*list_table)(struct hd_lowlevel_io_t *self);
	void		(*xlist_table)(struct hd_lowlevel_io_t *self);
	void		(*get_sectorsize)(struct hd_lowlevel_io_t *self);
	void		(*get_geometry)(struct hd_lowlevel_io_t *self,
				struct hd_geometry_t *g);
	void		(*guess_device_type)(struct hd_lowlevel_io_t *self);

	void		(*read_extended)(struct hd_lowlevel_io_t *self,
					int ext);
	void		(*read_pte)(struct hd_lowlevel_io_t *self,
					int pno, unsigned int offset);
	int		(*read_sector)(struct hd_lowlevel_io_t *self,
					unsigned int secno, char *buf);
	int		(*write_sector)(struct hd_lowlevel_io_t *self,
					unsigned int secno, char *buf);

	int		(*read_sectors)(struct hd_lowlevel_io_t *self,
					unsigned int start_sec,
					unsigned int num_sec,
					char *buffer);
	int		(*write_sectors)(struct hd_lowlevel_io_t *self,
					unsigned int start_sec,
					unsigned int num_sec,
					char *buffer);


	int		(*num_of_partitions)(struct hd_lowlevel_io_t *self);
	struct pte *	(*get_partition_entry)(struct hd_lowlevel_io_t *self,
				const int n);

	int		(*sector_size)(struct hd_lowlevel_io_t *self);
	int		(*cylinder_size)(struct hd_lowlevel_io_t *self);

	char *		(*partition_sysname)(struct hd_lowlevel_io_t *self,
				const int n);
};

struct hd_lowlevel_io_t	*new_hd_lowlevel_io (void);

#endif
