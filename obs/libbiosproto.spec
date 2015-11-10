#
# spec file for package libbiosproto
#
# Copyright (c) 2015 Eaton
#
# All modifications and additions to the file contributed by third parties
# remain the property of their copyright owners, unless otherwise agreed
# upon. The license for this file, and modifications and additions to the
# file, is the same license as for the pristine package itself (unless the
# license for the pristine package is not an Open Source License, in which
# case the license is the MIT License). An "Open Source License" is a
# license that conforms to the Open Source Definition (Version 1.9)
# published by the Open Source Initiative.

Name:           libbiosproto
Version:        0.0.1
Release:        1
License:        GPLv2+
Summary:        BIOS core protocols
Url:            http://eaton.com/
Group:          System/Libraries
Source0:        libbiosproto-%{version}.tar.gz
BuildRequires:  pkg-config
BuildRequires:  malamute-devel
BuildRoot:      %{_tmppath}/%{name}-%{version}-build

%description
Core protocols for BIOS

%package 0
Summary:        BIOS core protocols

%description 0
BIOS core protocols.
This package contains shared library.

%files 0
%defattr(-,root,root)
%doc COPYING
%{_libdir}/%{name}.so.*

%package devel
Requires:   %{name}0 = %{version}

%description devel
Core protocols for BIOS.
This package contains development files.

%files devel
%defattr(-,root,root)
%doc COPYING README.md
%{_includedir}/*.h
%{_libdir}/%{name}.so
%{_libdir}/pkgconfig/%{name}.pc

%prep
%setup -q

%build
%configure
make %{?_smp_mflags}

%install
make install DESTDIR=%{buildroot} %{?_smp_mflags}
find %{buildroot} -name '*.la' -or -name '*.a' -delete

%post -n %{name}0 -p /sbin/ldconfig

%postun -n %{name}0 -p /sbin/ldconfig

