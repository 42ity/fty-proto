#
# spec file for package libftyproto
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

%global soname %{name}0
Name:           libftyproto
Version:        0.0.1
Release:        1
License:        GPLv2+
Summary:        FTY core protocols
Url:            http://eaton.com/
Group:          System/Libraries
Source:         libftyproto-%{version}.tar.gz
BuildRequires:  pkg-config
BuildRequires:  malamute-devel
BuildRequires:  autoconf
BuildRequires:  automake
BuildRequires:  libtool
BuildRoot:      %{_tmppath}/%{name}-%{version}-build

%description
Core protocols for FTY

%package -n %{soname}
Summary:        Shared library for FTY core protocols

%description -n %{soname}
FTY core protocols.
This package contains shared library.

%files -n %{soname}
%defattr(-,root,root)
%doc COPYING
%{_libdir}/%{name}.so.*

%package devel
Summary:        Devel files for FTY core protocols
Requires:   %{soname} = %{version}

%description devel
Core protocols for FTY.
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
./autogen.sh
%configure
make %{?_smp_mflags}

%files
%doc README.md
%doc COPYING
%{_bindir}/generate_metric
%{_bindir}/get_metrics
%{_bindir}/bmsg

%install
make install DESTDIR=%{buildroot} %{?_smp_mflags}
find %{buildroot} -name '*.la' -or -name '*.a' | xargs rm -f

%post -n %{soname} -p /sbin/ldconfig

%postun -n %{soname} -p /sbin/ldconfig

%changelog
