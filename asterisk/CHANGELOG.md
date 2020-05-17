# CHANGELOG.md

The following file will serve as a record to explain the changes that have been made to the app_rpt and related
programs for the AllStar software. 

Released versions will correspond to ASL releases (e.g, ASL 1.02, etc.) and be lised below their respective sections.

Any changes/updates that hapen after a release will be listed under the Unreleased section.  Once a new release is made and those changes are merged in, they will be moved under the corresponding release header.

## Unreleased

No unrelesaed changes at this time.

##### Housekeeping
##### Internal code additions/modifications
##### Config file additions/modifications


## Released

### ASL 1.02 - 05/11/2020

#### app_rpt - v0.340 - 05/11/2020

##### Housekeeping

Cleaned up compile time warnings for app_rpt.c  These warnings have been around for a very long time and needed to be fixed.


##### Asterisk CLI additions/modifications

1.  Added rpt utils command with the following functions:

    a.  sayip <interface> <nodenum> - will use a version of LOCALPLAY that says the local IP address for the interface specified (if found).  By default will play on all nodes if no <nodenum> is specified.  Use 0 to turn off audio playback and just print IP address only.

    b.   pubip <nodenum> - like sayip but will query a remote site (default:  ifconfig.me/ip) to find out public IP address.  By default will play on all nodes if no <nodenum> is specified.  Use 0 to turn off audio playback and just print IP address only.

2.  Added rpt globals command with the following functions:

    a.  show - shows the settings from [globals] from rpt.conf stanza

    b.  set - allows setting of items from [globals] from rpt.conf stanza

      * maxlinks - change between 32 and 256.  If conslock > 0 cannot be set.
      * notchfilter - 0 = off/ 1=on
      * mdcencode - 0=off/ 1=on
      * mdcdecode - 0=off/ 1=on
      * localchannels - 0=off/ 1=on.  If conslock >0 cannot be set.
      * fakeserial - 0=off/ 1=on
      * noremotemdc - 0=off/ 1=on
      * nocdrpost - 0=off/ 1=on.  If conslost >0 cannot be set.
      * mdcsay - 0=off/ 1=on
      * linkclip - 0=off/ 1=on
      * zoption - 0=off/ 1=on
      *  alttune - 0=off/ 1=on
      * linkdtmf - 0=off/ 1=on
      * setic706ctcss - 0=off/ 1=on
      * dtmftimeout - 3 to 100

    c.  list - displays a listing of variables that can be manipulated using set


##### Internal code additions/modifications

1.  Added libcurl support.   Basic routines from func_curl.c for this feature.

2.  Curl user agent will include app_rpt vX.Y if version # present in tdesc var.  It will also include the Linux version from /proc/version
	asterisk-libcurl-agent/1.0 (app_rpt v0.340)(Linux version 4.9.0-8-amd64) or
	asterisk-libcurl-agent/1.0 (-)(Linux version 4.9.0-8-amd64) if version isn’t present in tdesc. 

3.  Added ALPHANUM_LOCAL to rpt telemetry state machine as a function.  ARB_ALPHA says (plays) alpha numeric strings over both local and remote audio on nodes (as well as sending telemetry data). ALPHANUM_LOCAL will say (play) whatever is passed just locally on nodes similar to what LOCALPLAY does without passing any telemetry.  Used by rpt utils [sayip|pubip] functions.

4.  Replaced conditional compilation of certain code with run time configurable ability to change or turn on/off options/code in app_rpt.  Configured via [globals] section in rpt.conf and using rpt globals on CLI.

    a.  maxlinks - change between 32 and 256.  Changes number of linked nodes enumerated/reported in telemetry status.  If conslock > 0 cannot be changed (set) via CLI.

    b.  notchfilter - 0 = off/ 1=on.   Turns notch filter on/off.

    c.  mdcencode - 0=off/ 1=on.  Turns mdeencode on/off.

    d.  mdcdecode - 0=off/ 1=on.  Turns mdedecode on/off.

    e.  localchannels - 0=off/ 1=on.  If conslock >0 cannot be changed (set) via CLI.  Turns the local channel type on/off.

    f.  fakeserial - 0=off/ 1=on.  Turns fakeserial (prints to console) on/off.


    g.  noremotemdc - 0=off/ 1=on.  Turns on/off remotemdc code.

    h.  nocdrpost - 0=off/ 1=on.  If conslock >0 cannot be changed (set) via CLI..  Turns CDR posting on/off.

    i.  mdcsay - 0=off/ 1=on. Turns mdcsay on/off.

    j.  linkclip - 0=off/ 1=on.  Turns linkclip on/off.

    k.  zoption - 0=off/ 1=on.  Enabled/disabled Z option in app_rpt.

    l.  alttune - 0=off/ 1=on.  Turns “alternate tune” function on/off.

    m.  linkdtmf - 0=off/ 1=on.  Turns linkdtmf on/off.

    n.  setic706ctcss - 0=off/ 1=on.  Turns CTCSS code for IC706 on/off.

    o.  dtmftimeout - 3 to ????.  Changes DTMF timeout.

5.  Changed how statpost reporting works.

    a.  Can be globally disabled for all nodes (statpost=0 in globals)

    b.  Can be set to use ASL stat server only (statpost=1 in globals - default)

    c.  Can be use to use custom statpost server URL (statpost=2 and statpost_url is set in globals)

    d.  Can be diabled for a single node (statpost_override=0 in node stanza)

    e.  Can be disabled globally (statpost=0 in globals) and enabled per node with statpost_override=1, statpost_custom=1, and statpost_url set in each node’s stanza.

   f.  By default does not report nodes under 2000 (private nodes) unless three conditions are met (statpost_url for node, statpost_override=128, statpost_custom=2 -- this is a per node setting in the node stanzas)

6.  Porting of DVSWITCH telemetry ducking port done by N4IRR and KC1KCC from the now defunct XiPAR release.  (https://github.com/AllStarLink/Asterisk/pull/53)

7.  Added three additional status commands for IP address reporting using DTMF tones over the air.

    a.  status,20 = Say remote (public) IP (global)

    b.  status,21 = Say remote (public) IP (local only)

    c.  status,22 = Say local network interface IP (local only)

8.  Replaced dependency on WGET in statpost with libcurl.  This eliminates the need to constantly fork a new copy of wget for each and every stats update.

9.  Fixed old issue with how PLAYBACK and LOCALPLAY work.  Per the app_rpt.c code, LOCALPLAY is supposed to play audio locally while PLAYBACK plays it globally. This was swapped, and use of LOCALPLAY ended up playing back audio globally.  LOCALPLAY is once again only for playing audio locally, while PLAYBACK plays it globally.

10. Set alignment of structures used in app_rpt.c to improve performance and memory utilization.


##### Config file additions/modifications

* /etc/asterisk/rpt.conf additions/changes/modifications

1.  Added [globals] stanza.

2.  Added the following config items to the [globals] stanza:

    a.  conslock - console lock for updating .  0=off (default), 1=lock maxlinks, nocdrpost, and localchannels, 2=lock all

    b.  maxlinks - changes MAXSTATLINKS.  Default is 32.  Max is 256.

    c.  notchfilter - 0 = off/ 1=on.   Turns notch filter on/off.  Default=0

    d.  mdcencode - 0=off/ 1=on.  Turns mdeencode on/off. Default=0

    e.  mdcdecode - 0=off/ 1=on.  Turns mdedecode on/off. Default=0

    f.  localchannels - 0=off/ 1=on.  If conslock >0 cannot be set.  Turns the local channel type on/off. Default=1

    g.  fakeserial - 0=off/ 1=on.  Turns fakeserial (prints to console) on/off. Default=0

    h.  noremotemdc - 0=off/ 1=on.  Turns on/off remotemdc code. Don't notify MDC1200 on remote bases.  Default=1

    i.  nocdrpost - 0=off/ 1=on.  If conslost >0 cannot be set..  Turns CDR posting on/off. Default=1

    j.  mdcsay - 0=off/ 1=on. Turns mdcsay on/off. MDC say when doing CT off - only works when MDC decode is enabled. Default=0

    k.  linkclip - 0=off/ 1=on.  Turns linkclip on/off. Code that causes clipping of first syllable on link. Default=0

    l.  zoption - 0=off/ 1=on.  Enabled/disabled Z option in app_rpt. Default=0

    m.  alttune - 0=off/ 1=on.  Turns “alternate tune” function on/off. Default=0

    n.  linkdtmf - 0=off/ 1=on.  Turns linkdtmf on/off. Extra link DTMF code. Default=0

    o.  setic706ctcss - 0=off/ 1=on.  Turns CTCSS code for IC706 on/off. Set IC706 CTCSS TX/RX Frequencies. Default=0

    p.  dtmftimeout - 3 to ????.  Changes DTMF timeout.  Default=3

    q.  remoteip_url - URL for fetching remote IP in rpt utils pubip (default:  http://ifconfig.me/ip)

    r.  statpost_url - Global statpost URL (default http://stats.allstarlink.org/uhandler.php)

    s.  statpost - Global control us turning update of stats via statpost_url on/off.  0=off, 1=Use AllStarLink (uses default and ignores statpost_url change), 2=use custom (statpost_url must be set)

3.  The following are changes to the individual node stanza configs:

    a.  Removed statpost_program.  Deprecated and removed due to use of libcurl now for stats reporting and [globals] section config options.

    b.  Individual node reporting can be disabled by setting statpost_override=0.

    c.  When global statpost reporting is disabled, individual node reporting can be enabled by setting statpost_override=1.

    d.  Changed individual node use of statpost_url.  All nodes will use the statpost_url defined in the [globals] section uless statpost_custom is set to 1.  Then the individual nodes can specify their own unique statport_url for use.

    e.  By default, nodes below 2000 (private nodes) will no longer be able to send updates to the statpost_url.  This behavior can be overridden on a per node basis by setting statpost_custom=2,statpost_override=128, and specifying a statpost_url.


#### chan_simpleusb.c - 05/11/2020

##### Housekeeping

Removed exta mutex unlock from code.


##### Internal code additions/modifications

Fixed audio support for CM119B chips.


##### Config file additions/modifications

1.  Added ctype option for device selection of audio. 0=auto; 1=C108; 2=C108AH; 3=N1KDO; 4=C119/C119A; 5=C119B.

2.  Added forceinit option to force USB device to attemp initilization. 0=off, 1=on. Must be used with ctype!


#### chan_usbradio.c - 05/11/2020

##### Housekeeping

N/A


##### Internal code additions/modifications

Fixed support for audio with CM119B.


##### Config file additions/modifications

N/A

