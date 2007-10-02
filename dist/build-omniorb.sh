#!/bin/sh


BLIB=false
BDEV=false
PREFIX=
HELP=false
DIST=
UNKNOWN=
REBUILD=true
TEST=false


function basename()
{
    local S=$IFS
    IFS="/"

    local bname=""
    local fname=""

    for f in $1 ; do
      if [ -z "$f" ]; then
	 continue
      fi
	
      if [ -z "$fname" ]; then
	  fname="$f"
          continue
      fi

      bname="$fname"
      fname="$fname/$f"
    done

    echo "$bname"
    IFS=$S;
}


function pkg_name()
{
    
    local my=$(echo $1 | sed -e 's/\(.\+\)-.*/\1/')
    local S=$IFS
    IFS="-"

    local name=""

    for f in $1 ; do
      name="$f"	
      break
    done
    
    IFS=$S;

    echo $name
}

function pkg_version()
{
    local my=$(echo $1 | sed -e 's/.\+-\([0-9]\+\.[0-9]\+\.[0-9]\+\)\..*/\1/')
    echo $my
}

function ziptype()
{
    local my=$(echo $1 | sed -e 's/.\+\.\(.*\)/\1/')
    echo $my
}

function use()
{
   echo " build-omniorb [-nobuild] [-test] -dev|-lib [-prefix myprefix] omniORB-X.Y.Z.tar.[bz|bz2|gz|tgz]"
   exit 1
}

while [ -n "$(echo $1 | grep  '^-')" ]; do
    case $1 in
    -lib     ) BLIB=true   ;;
    -dev     ) BDEV=true   ;;
    -prefix  ) PREFIX=$(echo $2) 
	       shift       ;;
    -test    ) TEST=true   ;;
    -nobuild ) REBUILD=false ;;
    -help    ) HELP=TRUE  ;;
    *        ) UNKNOWN=$1
	       echo "UNKNOWN: $1" ;;
    esac
    shift
done

DIST=$(echo $1)
NAME=$(pkg_name $DIST)
VERSION=$(pkg_version $DIST)
ZIPTYPE=$(ziptype $DIST)
TODIR=$(pwd)/$NAME-$VERSION-build
PKG=
TOP=$(pwd)

echo "BLIB:    $BLIB"
echo "BDEV:    $BDEV"
echo "PREFIX:  $PREFIX"
echo "DIST:    $DIST" 
echo "NAME:    $NAME"
echo "VERSION: $VERSION"
echo "ZIPTYPE: $ZIPTYPE"
echo "TODIR:   $TODIR"
echo "TOP:     $TOP"

if [ -z "$DIST" ]; then
    use
fi

if [ ! -f $DIST ]; then
    echo "File <$DIST> dont exist!"
    exit 1
fi

echo "Building: $DIST"

if [ "$BLIB" = "true" ]; then
    echo "A <lib> package!"
    PKG=lib
elif [ "$BDEV" = "true" ]; then 
    echo "A <dev> package!"
    PKG=dev
else
    use
fi


if [ "$PKG" = "lib" ]; then
    echo "Generating a LIB pkg!"
elif [ "$PKG" = "dev" ]; then
    echo "Generating a DEV pkg!"
fi

if [ "$TEST" = "true" ]; then
    exit 0;
fi

rm -rf $TODIR

if [ $REBUILD = "true" ]; then
    rm -rf $NAME-$VERSION

    if [ $ZIPTYPE = "bz" -o $ZIPTYPE = "bz2" ]; then
	tar jxpf $DIST
    elif [ $ZIPTYPE = "gz" -o $ZIPTYPE = "tgz" ]; then
	tar zxpf $DIST
    else
	echo "Unknown package format."
	echo "Expecting bz or gz!"
	use
    fi
fi
 
cd $NAME-$VERSION

if [ $REBUILD = "true" ]; then
    if [ "$?" -ne 0 ]; then
	"Cant change directory to: $NAME-$VERSION"
	exit 1
    fi
    
    if [ -f configure ]; then
	if [ ! -x configure ]; then
	    chmod +x configure
	    
	    if [ "$?" -ne 0 ]; then
		echo "Cant make <configure> executable!"
		exit 1
	    else
		echo "Changed <configure> to be executable!"
	    fi
	fi
    else
	echo "Strange: No <configure> file!"
	exit 1
    fi

    mkdir -p $TODIR/$PREFIX
    
    ./configure --prefix=$PREFIX \
	        --includedir=$PREFIX/include/omniorb          \
	        --with-omniORB-config=$PREFIX/etc/omniORB.cfg \
	        --with-omniNames-logdir=$PREFIX/var/omninames
    
    make
fi
 
make DESTDIR=$TODIR install

echo "$NAME-$PKG" >         $TODIR/$PREFIX/NAME
echo "$VERSION" > $TODIR/$PREFIX/VERSION


(cd $TODIR/$PREFIX; tar xpf $TOP/mipkg.tar)


if [ "$PKG" = "lib" ]; then
    rm -rf $TODIR/$PREFIX/bin/omkdepend
    rm -f  $TODIR/$PREFIX/bin/omnicpp 
    rm -f  $TODIR/$PREFIX/bin/omniidl  
    rm -f  $TODIR/$PREFIX/bin/omniidlrun.py 

    echo "bin/*"      >  $TODIR/$PREFIX/mipkg/files
    echo "lib/lib*"   >>  $TODIR/$PREFIX/mipkg/files

elif [ "$PKG" = "dev" ]; then
    echo "bin/*"            >  $TODIR/$PREFIX/mipkg/files
    echo "lib/lib*"        >>  $TODIR/$PREFIX/mipkg/files
    echo "lib/pkgconfig/*" >>  $TODIR/$PREFIX/mipkg/files
    echo "lib/python*/*"   >>  $TODIR/$PREFIX/mipkg/files
    echo "include/omniorb" >>  $TODIR/$PREFIX/mipkg/files
    echo "share/*"         >>  $TODIR/$PREFIX/mipkg/files
else
    echo "Which package shall be generated LIB or DEV!"
    use
fi

BASE=$(basename $TODIR/$PREFIX)
mv $TODIR/$PREFIX /$BASE/$NAME-$PKG-$VERSION

cd /$BASE

if [ "$?" -ne 0 ]; then
    echo "Cant change directory to <$BASE>!"
    exit 1
fi

tar cpf $NAME-$PKG-$VERSION.tar $NAME-$PKG-$VERSION
gzip $NAME-$PKG-$VERSION.tar

mv $NAME-$PKG-$VERSION.tar.gz $TOP

exit 0
