Name:    netrobots
Version: 0.1
Release: 0
Summary: netrobots, programming game with fighting robots
Group:   Games
License: GPLv2+
URL:     https://github.com/jbenc/netrobots
Source0: netrobots-%{version}.tar.bz2
BuildRequires: SDL-devel
Requires: SDL, netrobots-lib = %{version}

%description
Netrobots is a game where robots controlled by players' programs fight in
a battlefield.

%prep
%setup -q

%build
make %{?_smp_mflags}

%install
make install_all DESTDIR=%{buildroot}/ PREFIX=/usr
for f in counter rabbit rook sniper spot; do mv %{buildroot}/usr/bin/$f %{buildroot}/usr/bin/robot-$f; done

%files
%defattr(-,root,root,-)
/usr/bin/robotserver
/usr/bin/robot-counter
/usr/bin/robot-rabbit
/usr/bin/robot-rook
/usr/bin/robot-sniper
/usr/bin/robot-spot

%package lib
Group:   Games
Summary: C library for programming netrobots

%description lib
Netrobots is a game where robots controlled by players' programs fight in
a battlefield.

This package contains C library for robots programming.

%post lib -p /sbin/ldconfig

%postun lib -p /sbin/ldconfig

%files lib
%defattr(-,root,root,-)
/usr/lib/libnetrobots.so
/usr/include/netrobots.h
