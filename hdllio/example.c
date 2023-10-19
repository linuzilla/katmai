/*
 *	hdllio.c	(HD Low-Level I/O)
 *
 *	Copyright (c) 2003, Jiann-Ching Liu
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include "hdllio.h"

int main (int argc, char *argv[]) {
	struct hd_lowlevel_io_t		*hdio;
	struct hd_geometry_t		geom;
	int				i, cylsiz;
	struct pte			*pte;

	if ((hdio = new_hd_lowlevel_io ()) == NULL) {
		perror ("new_hd_lowlevel_io");
		return 1;
	}


	if (hdio->open (hdio, "/dev/hda") > 0) {
		cylsiz = hdio->cylinder_size (hdio);
		hdio->get_geometry (hdio, &geom);
		fprintf (stderr, "C=%d,H=%d,S=%d (%d)[%d Bytes/Sector](%d)\n",
				geom.cylinders, geom.heads, geom.sectors,
				hdio->num_of_partitions (hdio),
				hdio->sector_size (hdio),
				cylsiz);

		for (i = 0; i < hdio->num_of_partitions (hdio); i++) {
			pte = hdio->get_partition_entry (hdio, i);
			fprintf (stderr,
				"%2d. %8u %8u (%8u) [%02x] %6d %6d %s\n",
					i + 1, pte->start_sector,
					pte->num_of_sectors,
					pte->offset,
					pte->part_table->sys_ind,
					pte->start_sector / cylsiz + 1,
					(pte->start_sector +
					 pte->num_of_sectors) / cylsiz,
					hdio->partition_sysname (hdio, i));
		}
		// hdio->list_table (hdio);
		hdio->close (hdio);
	}

	return 0;
}
