Summary: UCL Robust-Audio Tool
Name: rat
Version: 4.0.4
Release: 1
Source: http://www-mice.cs.ucl.ac.uk/multimedia/software/rat-4.0/4.0.4/rat-4.0.4.tar.gz
Copyright: Copyright (c) 1995-99 University College London
Group: X11/Applications/Networking
Packager: Colin Perkins <c.perkins@cs.ucl.ac.uk>
Summary: RAT - unicast and multicast voice-over-IP application

%description
RAT is the premier open source voice-over-IP
application. It allows users to particpate in audio
conferences over the internet. These can be between
two participants directly, or between a group of
participants on a common multicast group. No special
features are required to use RAT in point-to-point
mode, but to use the multiparty conferencing facilities
of RAT, all participants must reside on a portion of the
Internet which supports IP multicast. RAT is based on
IETF standards, using RTP above UDP/IP as its
transport protocol, and conforming to the RTP profile
for audio and video conference with minimal control. 

RAT features sender based loss mitigation mechanisms
and receiver based audio repair techniques to compensate 
for packet loss, and load adaption in response to host 
performance. It runs on a range of platforms: FreeBSD, 
HP-UX, IRIX, Linux, NetBSD, Solaris, SunOS, and Windows 
95/NT. The source code is publicly available for porting 
to other platforms and for modification by others. 

Note that RAT does not perform call services like user 
location, neither does it listen to session announcements 
to discover advertised multicast sessions. For these 
purposes, it is recommended you use RAT in conjunction 
with the Session Directory (SDR), or a similar application. 

See http://www-mice.cs.ucl.ac.uk/multimedia/software/rat-4.0/

%prep

%setup

%build

cd ../tcl-8.0/unix
./configure
make
cd ../../tk-8.0/unix
./configure
make
cd ../../common
./configure
make
cd ../rat
./configure
make
cd ..

%install

install -m 755 rat/rat-4.0.4 		/usr/bin/rat-4.0.4
install -m 644 rat/README    		/usr/doc/rat-4.0.4/README
install -m 644 rat/README.devices	/usr/doc/rat-4.0.4/README.devices
install -m 644 rat/README.files		/usr/doc/rat-4.0.4/README.files
install -m 644 rat/README.gsm		/usr/doc/rat-4.0.4/README.gsm
install -m 644 rat/README.mbus		/usr/doc/rat-4.0.4/README.mbus
install -m 644 rat/README.playout	/usr/doc/rat-4.0.4/README.playout
install -m 644 rat/README.timestamps	/usr/doc/rat-4.0.4/README.timestamps
install -m 644 rat/man/man1/rat.1	/usr/man/man1/rat.1

%files 
/usr/bin/rat-4.0.4
/usr/doc/rat-4.0.4/README
/usr/doc/rat-4.0.4/README.devices
/usr/doc/rat-4.0.4/README.files
/usr/doc/rat-4.0.4/README.gsm
/usr/doc/rat-4.0.4/README.mbus
/usr/doc/rat-4.0.4/README.playout
/usr/doc/rat-4.0.4/README.timestamps
/usr/man/man1/rat.1

