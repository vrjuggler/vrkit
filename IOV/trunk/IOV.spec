# Spec file for IOV.
%define name    IOV
%define version	0.1.0
%define release	1

Name: %{name}
Summary: IOV
Version: %{version}
Release: %{release}
Source: %{name}-%{version}.tar.bz2
URL: http://www.infiscape.com
Group: System Environment/Libraries
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-buildroot
License: Copyright Infiscape Corporation

Requires: vrjuggler >= 2.0-beta3
Requires: OpenSG >= 1.4.0-1
Requires: boost >= 1.32
Requires: cppdom >= 0.3.3

BuildRequires: vrjuggler-devel >= 2.0-beta3
BuildRequires: OpenSG-devel >= 1.4.0-1
BuildRequires: boost-devel >= 1.32
BuildRequires: cppdom-devel >= 0.3.3
BuildRequires: gmtl >= 0.4.5

Provides: IOV = %{version}-%{release}
Obsoletes: IOV <= %{version}-%{release}

AutoReqProv: no

%description
IOV

%package devel
Summary: The IOV Headers
Group: System Environment/Libraries
Requires: IOV = %{version}-%{release}
Provides: IOV-devel = %{version}-%{release}

%description devel
Headers for IOV

%prep
rm -rf $RPM_BUILD_ROOT
%setup -q
#%setup -DT -q

%build
# Build and install
scons optimize=yes BoostBaseDir=/usr BoostIncludeDir=/usr/include VrjBaseDir=/usr OpenSGBaseDir=/usr prefix=$RPM_BUILD_ROOT/usr/local/infiscape install

%install
# Remove all stupid scons temp files
find $RPM_BUILD_ROOT/usr/local/infiscape -name .sconsign -exec rm {} \;

%clean
rm -rf $RPM_BUILD_ROOT

%pre

%post

%preun

%postun

%files
%defattr(-, root, root)
/usr/local/infiscape/lib/libiov.so
/usr/local/infiscape/lib/IOV
/usr/local/infiscape/share

%files devel
%defattr(-, root, root)
/usr/local/infiscape/include/IOV

#%doc README AUTHORS ChangeLog COPYING

%changelog
