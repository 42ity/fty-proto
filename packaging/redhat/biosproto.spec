#
#    biosproto - Core protocols for BIOS
#
#    Copyright (C) 2014 - 2015 Eaton                                        
#                                                                           
#    This program is free software; you can redistribute it and/or modify   
#    it under the terms of the GNU General Public License as published by   
#    the Free Software Foundation; either version 2 of the License, or      
#    (at your option) any later version.                                    
#                                                                           
#    This program is distributed in the hope that it will be useful,        
#    but WITHOUT ANY WARRANTY; without even the implied warranty of         
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          
#    GNU General Public License for more details.                           
#                                                                           
#    You should have received a copy of the GNU General Public License along
#    with this program; if not, write to the Free Software Foundation, Inc.,
#    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.            
#

Name:           biosproto
Version:        0.1.0
Release:        1
Summary:        core protocols for bios
License:        MIT
URL:            http://example.com/
Source0:        %{name}-%{version}.tar.gz
Group:          System/Libraries
BuildRequires:  automake
BuildRequires:  autoconf
BuildRequires:  libtool
BuildRequires:  pkg-config
BuildRequires:  systemd-devel
BuildRequires:  libsodium-devel
BuildRequires:  zeromq-devel
BuildRequires:  uuid-devel
BuildRequires:  czmq-devel
BuildRequires:  malamute-devel
BuildRoot:      %{_tmppath}/%{name}-%{version}-build

%description
biosproto core protocols for bios.

%package -n libbiosproto0
Group:          System/Libraries
Summary:        core protocols for bios

%description -n libbiosproto0
biosproto core protocols for bios.
This package contains shared library.

%post -n libbiosproto0 -p /sbin/ldconfig
%postun -n libbiosproto0 -p /sbin/ldconfig

%files -n libbiosproto0
%defattr(-,root,root)
%doc COPYING
%{_libdir}/libbiosproto.so.*

%package devel
Summary:        core protocols for bios
Group:          System/Libraries
Requires:       libbiosproto0 = %{version}
Requires:       libsodium-devel
Requires:       zeromq-devel
Requires:       uuid-devel
Requires:       czmq-devel
Requires:       malamute-devel

%description devel
biosproto core protocols for bios.
This package contains development files.

%files devel
%defattr(-,root,root)
%{_includedir}/*
%{_libdir}/libbiosproto.so
%{_libdir}/pkgconfig/libbiosproto.pc

%prep
%setup -q

%build
sh autogen.sh
%{configure} --with-systemd
make %{_smp_mflags}

%install
make install DESTDIR=%{buildroot} %{?_smp_mflags}

# remove static libraries
find %{buildroot} -name '*.a' | xargs rm -f
find %{buildroot} -name '*.la' | xargs rm -f

%files
%defattr(-,root,root)
%doc README.md
%doc COPYING
%{_bindir}/generate_metric
%{_bindir}/get_metrics
%{_bindir}/bmsg


%changelog
