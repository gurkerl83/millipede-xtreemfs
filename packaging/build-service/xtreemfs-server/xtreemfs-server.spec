Name:           XtreemFS-server
Version:        _VERSION_
Release:        1
License:        GPL
BuildRoot:      %{_tmppath}/%{name}-%{version}-build
Group:          Networking
Summary:        XtreemFS server components (DIR, MRC, OSD)
Source0:        %{name}-%{version}.tar.gz
Requires:       grep sudo

# openSUSE >=10.2 
%if 0%{?suse_version} >= 1020 
BuildRequires:  pwdutils >= 3
%endif

%description
XtreemFS is a distributed and replicated file system for the internet. For more details, visit www.xtreemfs.org.

This package contains the XtreemFS server components (DIR, MRC, OSD).
To run the XtreemFS services, a SUN JAVA 6 RUNTIME ENVIROMENT IS REQUIRED! Make sure that Java is installed in /usr/bin, or $JAVA_HOME is set.

%prep
%setup

%build

%install
XTREEMFS_JAR_DIR=$RPM_BUILD_ROOT/usr/share/java/
XTREEMFS_CONFIG_DIR=$RPM_BUILD_ROOT/etc/xos/xtreemfs/
XTREEMFS_INIT_DIR="$RPM_BUILD_ROOT/etc/init.d/"
BIN_DIR="$RPM_BUILD_ROOT/usr/bin"
MAN_DIR="$RPM_BUILD_ROOT/usr/share/man"
DOC_DIR="$RPM_BUILD_ROOT/usr/share/doc/xtreemfs-server"

export NO_BRP_CHECK_BYTECODE_VERSION=true

# copy copyright notes
mkdir -p $DOC_DIR
cp COPYING $DOC_DIR

# copy jars
mkdir -p $XTREEMFS_JAR_DIR
cp dist/XtreemFS.jar $XTREEMFS_JAR_DIR
cp lib/*.jar $XTREEMFS_JAR_DIR

# copy config files
mkdir -p $XTREEMFS_CONFIG_DIR
cp config/*.properties $XTREEMFS_CONFIG_DIR
# copy generate_uuid script
cp packaging/generate_uuid $XTREEMFS_CONFIG_DIR
cp packaging/postinstall_setup.sh $XTREEMFS_CONFIG_DIR
chmod a+x $XTREEMFS_CONFIG_DIR/postinstall_setup.sh

# copy init.d files
mkdir -p $XTREEMFS_INIT_DIR
cp init.d-scripts/xtreemfs-* $XTREEMFS_INIT_DIR
chmod a+x $XTREEMFS_INIT_DIR/xtreemfs-*

%post
XTREEMFS_CONFIG_DIR=/etc/xos/xtreemfs/

# generate UUIDs
if [ -x $XTREEMFS_CONFIG_DIR/generate_uuid ]; then
  $XTREEMFS_CONFIG_DIR/generate_uuid $XTREEMFS_CONFIG_DIR/dirconfig.properties
  $XTREEMFS_CONFIG_DIR/generate_uuid $XTREEMFS_CONFIG_DIR/mrcconfig.properties
  $XTREEMFS_CONFIG_DIR/generate_uuid $XTREEMFS_CONFIG_DIR/osdconfig.properties
else
  echo "UUID can't be generated automatically. Please enter a correct UUID in each config file of a xtreemfs service."
fi

$XTREEMFS_CONFIG_DIR/postinstall_setup.sh

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
/usr/share/java/*.jar
/etc/init.d/xtreemfs-*
/etc/xos/
%config(noreplace) /etc/xos/xtreemfs/*.properties
/etc/xos/xtreemfs/generate_uuid
/etc/xos/xtreemfs/postinstall_setup.sh
/usr/share/doc/xtreemfs-server/

%changelog
