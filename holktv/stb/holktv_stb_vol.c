/*
 *	holktv_stb_vol.c
 *
 *	Copyright (c) 2003, Jiann-Ching Liu
 */

static void hktv_set_system_volume (struct holktv_stb_t *self) {
	struct holktv_stb_pd_t	*pd = &self->pd;
	int			i;

	pd->mixer->close_lock (1);
	// fprintf (stderr, "[+] Volume: [");
	for (i = 0; i < VOLUME_LEVEL_SOURCE; i++) {
		if (volume_mapping[i] >= 0) {
			if (volume_mask.src[i] == 0) {
				// fprintf (stderr, " -");
				pd->mixer->setvolume (volume_mapping[i], 0);
			} else {
				// fprintf (stderr, " %d",
				//		pd->volume_setting.src[i]);
				pd->mixer->setvolume (volume_mapping[i],
						pd->volume_setting.src[i]);
			}
		}
	}
	// fprintf (stderr, " ]\n");
	pd->mixer->close_lock (0);
	pd->mixer->close ();
}

static void hktv_volume_switch (struct holktv_stb_t *self,
					const int on, const int bitmap) {
	struct holktv_stb_pd_t	*pd = &self->pd;
	int			i;
	int			mask = 1;
	int			v;

	pd->mixer->close_lock (1);

	for (i = VOLUME_LEVEL_SOURCE - 1; i >= 0; i--) {
		if (volume_mapping[i] >= 0) {
			if ((bitmap & mask) != 0) {
				v = on ? pd->volume_setting.src[i] : 0;
				pd->mixer->setvolume (volume_mapping[i], v);
			}
		}

		mask <<= 1;
	}

	pd->mixer->close_lock (0);
	pd->mixer->close ();
}

static void hktv_cdpcm (struct holktv_stb_t *self,
					const int cd, const int pcm) {
	int		flag;

	flag = 0;

	if (! cd)  flag |= 4;
	if (! pcm) flag |= 2;
	if (flag != 0) self->volume_switch (self, 0, flag);

	flag = 0;
	if (cd)  flag |= 4;
	if (pcm) flag |= 2;
	if (flag != 0) self->volume_switch (self, 1, flag);
}

static int hktv_get_volume_level (struct holktv_stb_t *self, char *str) {
	int			rc = 0;
	struct volume_level_t	*vol = &self->pd.volume_setting;

	rc = sprintf (str, "%d,%d,%d,%d,%d,%d,%d",
			vol->src[0],
			vol->src[1],
			vol->src[2],
			vol->src[3],
			vol->src[4],
			vol->src[5],
			vol->src[6]);
	return rc;
}

static int hktv_set_volume_level (struct holktv_stb_t *self, const char *str) {
	int			i, j, v, s;
	struct volume_level_t	vol;
	int			len;
	int			changed = 0;

	len = strlen (str);

	memcpy (&vol, &self->pd.volume_setting, sizeof vol);

	for (i = j = v = s = 0; i < len; i++) {
		if (str[i] == ',') {
			if (s != 0) vol.src[j] = v;
			s = v = 0;
			if (++j >= 7) break;
		} else if ((str[i] >= '0') && (str[i] <= '9')) {
			v = v * 10 + (int) (str[i] - '0');
			s = 1;
		} else {
			// fprintf (stderr, "Unexpected parameter\n");
			return 0;
		}
	}

	if ((s != 0) && (j < 7)) vol.src[j] = v;

	// fprintf (stderr, "Volume:");
	for (i = 0; i < VOLUME_LEVEL_SOURCE; i++) {
		if (vol.src[i] != self->pd.volume_setting.src[i]) {
			// fprintf (stderr, " %2d", vol.src[i]);
			changed += i;
		// } else {
			// fprintf (stderr, " --");
		}
	}
	// fprintf (stderr, "\n");

	memcpy (&self->pd.volume_setting, &vol, sizeof vol);

	if (changed) self->set_system_volume (self);

	return 1;
}

static int hktv_load_volume (struct holktv_stb_t *self, const char *file) {
	int	fd;

	if ((fd = open (file, O_RDONLY)) < 0) {
		perror (file);
		return 0;
	}

	read (fd, &self->pd.volume_setting, sizeof self->pd.volume_setting);
	close (fd);

	self->set_system_volume (self);
	return 1;
}

static int hktv_save_volume (struct holktv_stb_t *self, const char *file) {
	int	fd;

	if ((fd = open (file, O_RDWR|O_TRUNC|O_CREAT, 0644)) < 0) {
		perror (file);
		return 0;
	}

	write (fd, &self->pd.volume_setting, sizeof self->pd.volume_setting);
	close (fd);

	return 1;
}
