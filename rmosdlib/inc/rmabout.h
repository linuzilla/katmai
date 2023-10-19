
// the equivalent of RMver.h under windows NT/9x drivers
// used is FMPabout()


// RM_VERSION is the major version number
// RM_REVISION is the minor version number
// RM_BUILD is the build number. This one always increases.
// RM_RELEASE is the minor build number. If the build has failed, the minor build number is increased on the next 
// build attempt.

#define RM_VERSION      1
#define RM_REVISION     3
#define RM_BUILD        60 
#define RM_RELEASE      0

#define RM_COMPANYNAME          TEXT("Sigma Designs Inc.")
#define RM_LEGALCOPYRIGHT       TEXT("Copyright Sigma Designs Inc. © 2001")
#define RM_PRODUCTNAME          TEXT("REALmagic MPEG Decoders")


// Here is how to format the entry :
// Build number : Date : Time : Name of the project and OS.

// 0.1.005.0 : May 23 2000 : 17:20PST : Progressive Player Debug Board Build	(vxWorks x86)
// 0.1.006.0 : May 24 2000 : 17:15PST : Progressive Player Debug Board Build	(vxWorks x86)
// 0.1.007.0 : Jun 05 2000 : 16:12PST : vxWorks builds : DVD (8400), streaming (8400 + 8220), Liberate (8220 + 8400) (vxWorks x86)
// 0.1.008.0 : Jun 07 2000 : 17:30PST : Progressive Player Debug Board Build (vxWorks x86)
// 0.1.009.0 : Jun 23 2000 : 17:05PST : AutoPC Clarion Cabo D (Windows CE x86)
// 0.1.010.0 : Jun 27 2000 : 20:07CET : beta-12 for Linux with SVCD among other novelties
// 0.1.011.0 : Jun 28 2000 : 16:00PST : AutoPC Clarion Cabo D (Windows CE x86)
// 0.1.012.0 : Jun 29 2000 : 17:55PST : Progressive Player Debug Board Build	(vxWorks x86)
// 0.1.013.0 : Jul 06 2000 : 18:16PST : AutoPC Clarion Cabo D (Windows CE x86)
// 0.1.014.0 : Jul 07 2000 : 20:27PST : Progressive Player Debug Board Build	(vxWorks x86)
// 0.1.015.0 : Jul 12 2000 : 20:27PST : SVCD support / Galaxy II output selection added ( Linux x86 )
// 0.1.016.0 : Jul 13 2000 : 15:50PST : vxWorks build for Boca 2020 EM8400 (vxWorks x86)
// 0.1.017.0 : Jul 13 2000 : 19:45PST : Progressive Player Debug Board Build	(vxWorks x86)
// 0.1.018.0 : Jul 21 2000 : 10:26PST : Progressive Player Debug Board Build	(vxWorks x86)
// 0.1.019.0 : Jul 26 2000 : 20:07CET : Linux EM8400 driver
// 0.1.020.0 : Jul 27 2000 : 16:00PST : Progressive Player Debug Board Build	(vxWorks x86)
// 0.1.021.0 : Jul 28 2000 : 11:05PST : vxWorks build for Boca 2020 EM8400 (vxWorks x86)
// 0.1.022.0 : Aug  2 2000 :          : Linux EM8400 driver build
// 0.1.023.0 : Aug  4 2000 : 16:30CET : Linux EM8400 driver build. region broken
// 0.1.024.0 : Aug  4 2000 : 20:00CET : Linux EM8400 driver build.
// 0.1.025.0 : Aug  7 2000 : 19:30PST : Progressive Player Debug Board Build	(vxWorks x86)
// 0.1.026.0 : Aug 11 2000 : 11:30PST : Progressive Player Debug Board Build	(vxWorks x86)
// 0.1.027.0 : Aug 28 2000 : 11:55CET : Linux EM8400 driver build (bogus)
// 0.1.028.0 : Aug 29 2000 : 15:05CET : Linux EM8400 driver build (bogus)
// 0.1.029.0 : Aug 30 2000 : 20:00CET : Linux EM8400 driver build
// 0.1.030.0 : Aug 31 2000 : 16:30PST : AutoPC Clarion Cabo D (Windows CE x86)
// 0.1.031.0 : Sep  1 2000 : 16:00PST : vxWorks builds : Liberate (8220 + 8400) (vxWorks x86)
// 0.1.032.0 : Sep  1 2000 : 20:00PST : Progressive Player Debug Board Build	(vxWorks x86)
// 0.1.033.0 : Sep  4 2000 : 14:30CET : Linux EM8400 driver build
// 0.2.034.0 : Oct  6 2000 : 10:15PST : Progressive Player Debug Board Build + Princeton (vxWorks x86)
// 0.2.034.1 : Oct  9 2000 : 17:30PST : Windows CE general release (AutoPC, Generic 3.00, 2.12, Venus, ...) (Canceled)
// 0.2.034.2 : Oct 10 2000 : 11:15PST : Windows CE general release (AutoPC, Generic 3.00, 2.12, Venus, ...)
// 0.2.034.3 : Oct 17 2000 : 20:11CET : Linux EM8400 driver build; first one with analog overlay
// 0.2.034.4 : Oct 20 2000 : 22:25CET : Linux EM8400 driver build
// 0.2.035.0 : Oct 25 2000 : 16:25CET : Linux EM8400 driver build; first one with celeste app
// 0.2.036.0 : Oct 30 2000 : 12:30PST : Progressive Player Debug Board Build + Princeton (vxWorks x86)
// 0.2.036.1 : Nov 28 2000 : 12:10CET : Linux EM8400 driver build
// 0.2.037.0 : Nov 30 2000 : 16:44CET : Linux EM8400 driver build
// 0.2.037.1 : Dec 18 2000 : 11:30CET : Linux EM8400 driver build
// 0.2.038.0 : Jan 03 2001 : 14:45PST : Linux EM8400 driver build
// 0.2.038.1 : Jan 03 2001 : 18:25PST : Linux EM8400 driver build
// 0.2.039.0 : Jan 09 2001 : 17:00PST : vxWorks build for Boca 2020 EM8400 (vxWorks x86)
// 0.2.040.0 : Jan 11 2001 : 16:00PST : vxWorks build for Boca 2020 EM8400 (vxWorks x86)
// 0.2.040.1 : Jan 12 2001 : 11:25PST : Linux EM8400 driver build (National OEM)
// 0.2.041.0 : Jan 16 2001 : 16:00PST : Linux EM8400 driver build
// 0.2.042.0 : Feb  9 2001 : 16:00PST : Linux EM8400 driver build -- first attempt
// 0.2.042.1 : Feb 23 2001 : 15:00CET : Linux EM8400 driver build -- second attempt
// 0.2.042.2 : Feb 23 2001 : 16:00PST : Linux EM8400 driver build -- third attempt
// 0.2.043.00 : Mar 16 2001 : 20:00PST : vxWorks build for Boca 2020 EM8400 (vxWorks x86)
// 0.2.044.00 : May 02 2001 : 11:00PST : Linux EM8400 driver build 
// 0.2.045.00 : Mar 29 2001 : 12:00PST : vxWorks build for Boca i2020 EM8400 (vxWorks x86)
// 0.2.046.00 : Jun 22 2001 : 18:00CET : Linux EM8400 driver build by Emmanuel
// 0.2.046.01 : Jun 22 2001 : 19:10CET : Linux EM8400 driver build by Emmanuel
// 0.2.047.00 : Jul  6 2001 : 11:40CET : Linux EM8400 driver build by Emmanuel
// 0.2.047.01 : Jul  6 2001 : 11:50CET : Linux EM8400 driver build by Emmanuel
// 0.2.047.02 : Jul 19 2001 : 19:05CET : Linux EM8400 driver build by Emmanuel
// 0.2.048.01 : Jul 22 2001 : 19:05CET : vxWorks build for Boca (iDVD) i2020 EM8400 (vxWorks x86) (Frederic)
// 0.2.048.02 : Jul 24 2001 : 19:05CET : vxWorks build for Boca (iDVD) i2020 EM8400 (vxWorks x86) (Frederic)
// 0.2.048.03 : Jul 25 2001 : 19:05CET : vxWorks build for Boca (iDVD) (Increased tCallBack stack) i2020 EM8400 (vxWorks x86) (Frederic)
// 0.2.048.04 : Jul 27 2001 : 19:05CET : vxWorks build for Boca (iDVD) (Increased tSource stack) i2020 EM8400 (vxWorks x86) (Frederic)
// 0.2.048.05 : Sep  5 2001 : 19:00PST : Linux EM8400 driver build (Pascal)
// 0.2.048.06 : Sep  6 2001 : 20:10PST : Linux EM8400 driver build (Pascal)
// 0.2.048.07 : Sep 20 2001 : 13:44CET : Linux EM8400 driver build (Emmanuel)
// 0.2.048.08 : Sep 20 2001 : 15:00CET : Linux EM8400 driver build (Emmanuel)
// 0.2.048.09 : Sep 20 2001 : 16:44CET : Linux EM8400 driver build (Emmanuel)
// 0.2.048.10 : Sep 20 2001 : 18:40CET : Linux EM8400 driver build (Emmanuel)
// 0.2.048.11 : Oct 11 2001 : 17:43PST : vxWorks iDVD driver build (fixed the audio choking problem)
// 0.2.048.12 : Oct 22 2001 : 17:43PST : vxWorks iDVD driver build (new cdda navigation)
// 0.2.049.00 : Nov 5 2001 : 16:45EST : Linux EM8400 driver build (Michael)
// 0.2.049.01 : Nov 5 2001 : 20:00EST : Linux EM8400 driver build (Michael) (STC problem)
// 0.2.049.02 : Nov 7 2001 : 16:52CET : Tortuga build (Emmanuel)
// 0.2.049.03 : Nov 7 2001 : 13:00EST : Linux EM8400 driver build (Michael)
// 0.2.049.04 : Nov 7 2001 : 18:58CET : Tortuga build (Emmanuel)
// 0.2.049.05 : Nov 7 2001 : 18:00EST : Linux EM8400 driver build (Michael)
// 0.2.049.06 : Nov 8 2001 : 11:57CET : Linux EM8400 driver build (Emmanuel)
// 0.2.049.07 : Nov 9 2001 : 11:46CET : Linux EM8400 driver build (Emmanuel)
// 0.2.049.08 : Nov 19 2001 : 19:06CET : CARIBBEAN build (Emmanuel)
// 0.2.050.07 : Nov 15 2001 : 16:45PST : Windows CE general release (Generic 3.00, 2.12...)
// 0.2.050.08 : Nov 27 2001 : 11:40CET : Linux EM8400 driver build (Emmanuel)
// 0.2.050.09 : Nov 27 2001 : 14:10CET : Linux EM8400 driver build (Emmanuel)
// 0.2.050.10 : Nov 28 2001 : 10:39CET : Tortuga build (Emmanuel)
// 0.2.050.11 : Nov 28 2001 : 10:45CET : Tortuga build (Emmanuel)
// 0.2.050.12 : Nov 28 2001 : 11:09CET : Tortuga build (Emmanuel)
// 0.2.051.00 : Dec 7 2001 : 15:25EST : Linux EM8400 driver build (Michael)
// 0.2.051.01 : Dec 11 2001 : 15:25EST : CARIBBEAN build (Julien)
// 0.2.052.00 : Dec 16 2001 : 15:25EST : imprimis build (vincent)
// 1.0.052.00 : Dec 21 2001 : 20:41CET : CARIBBEAN build (Emmanuel)
// 1.1.0.0    : Jan 11 2002 : 14:33CET : ANTILLES build (Emmanuel)
// 1.1.0.1    : Jan 15 2002 : 22:51PST : imprimis build (vincent)
// 1.01.054.00 : Jan 16 2002 : 16:15PST : resync number : the next number will be : 1.01.055.00 (michael)
// 1.01.055.00 : Jan 19 2002 : 23:22PST : imprimis build (vincent)
// 1.01.056.00 : Jan 30 2002 : 17:48PST : imprimis build (vincent)
// 1.01.057.00 : Jan 31 2002 : 14:00PST : imprimis build (vincent)
// 1.01.058.00 : Feb 1 2002 : 16:05CET : linux EM8400 driver build (Emmanuel)
// 1.03.059.00 : Feb 27 2002 : 17:00EST : linux EM8400 driver build (Emmanuel)
// 1.03.060.00 : Feb 28 2002 : 12:00EST : linux EM8400 driver build (Michael)
