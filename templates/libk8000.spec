Summary: A Velleman K8000 C/C++ interface library for user-space programs.
Name: libk8000
Version: 2.2.0
Release: 1
License: GPL
Distribution: Kweetnie
Group: System Environment/Libraries
Source: http://struyve.mine.nu:8080/other/libk8000-2.2.0.tar.gz
URL: http://struyve.mine.nu:8080/
Vendor: Joris Struyve <joris@struyve.be>
Packager: joris Struyve <joris@struyve.be>

%description
This is a C/C++ library to interface with a Velleman K8000
computer interface board from a user-space program.
You can also talk to other I2C (like LCD panels) devices
that have their I2C wires connected to a K8000.

%prep
rm -fr  $RPM_BUILD_DIR/lib8000
tar -zxvf $RPM_SOURCE_DIR/libk8000-2.2.0.tar.gz

%build
cd $RPM_BUILD_DIR/libk8000
make

%install
cd $RPM_BUILD_DIR/libk8000
make install
mkdir -p /usr/share/doc/libk8000-2.2.0
install -m 644 $RPM_BUILD_DIR/libk8000/README /usr/share/doc/libk8000-2.2.0
install -m 644 $RPM_BUILD_DIR/libk8000/ChangeLog /usr/share/doc/libk8000-2.2.0
install -m 644 $RPM_BUILD_DIR/libk8000/COPYING /usr/share/doc/libk8000-2.2.0
install -m 644 $RPM_BUILD_DIR/libk8000/TODO /usr/share/doc/libk8000-2.2.0


%files
/usr/lib/libk8000.a
/usr/include/k8000.h
/usr/share/doc/libk8000-2.2.0/README
/usr/share/doc/libk8000-2.2.0/ChangeLog
/usr/share/doc/libk8000-2.2.0/COPYING
/usr/share/doc/libk8000-2.2.0/TODO

