
#!/bin/sh


BLIB=false
BDEV=false
PREFIX=
HELP=false
DIST=
UNKNOWN=
NOBUILD=false
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
    local my=$(echo $1 | sed -e 's/perl-\([0-9]\+\.[0-9]\+\.[0-9]\+\)\..*/\1/')
    echo $my
}

function ziptype()
{
    local my=$(echo $1 | sed -e 's/.\+\.\(.*\)/\1/')
    echo $my
}

function use()
{
   echo " build-perl [-test] -dev|-lib [-prefix myprefix] perl-X.Y.Z.tar.[bz|bz2|gz|tgz]"
   exit 1
}

while [ -n "$(echo $1 | grep  '^-')" ]; do
    case $1 in
    -lib    ) BLIB=true    ;;
    -dev    ) BDEV=true    ;;
    -prefix ) PREFIX=$(echo $2) 
	      shift        ;;
    -test   ) TEST=true    ;;
    -nobuild) NOBUILD=true ;;
    -help    ) HELP=TRUE   ;;
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


if [ "$NOBUILD" = false ]; then
    rm -rf $NAME-$VERSION
fi 

rm -rf $TODIR

if [ "$NOBUILD" = false ]; then
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

cd $NAME-$VERSION || (echo "Cant change directory to: $NAME-$VERSION"; exit 1)

if [ "$NOBUILD" = false ]; then
    if [ -f Configure ]; then
	if [ ! -x Configure ]; then
	    chmod +x Configure || \
		(echo "Cant make <Configure> executable!"; exit 1)

	    echo "Changed <Configure> to be executable!"
	fi
	./Configure -Dcc=gcc -Dprefix=$PREFIX -Dusethreads -des
    else
	echo "Strange: No <Configure> file!"
	exit 1
    fi
fi

mkdir -p $TODIR/$PREFIX
make DESTDIR=$TODIR install

rm -rf $TODIR/$PREFIX/man

echo "$NAME-$PKG" >         $TODIR/$PREFIX/NAME
echo "$VERSION" > $TODIR/$PREFIX/VERSION


(cd $TODIR/$PREFIX; tar xpf $TOP/mipkg.tar)


if [ "$PKG" = "lib" ]; then
    echo "lib/perl5/$VERSION" >  $TODIR/$PREFIX/mipkg/files
    rm -rf $TODIR/$PREFIX/bin
elif [ "$PKG" = "dev" ]; then
#    CONFIGTMP=$TODIR/$PREFIX/bin/kvperl-config.tmp
#
#    echo "#! /bin/sh"  >  $CONFIGTMP
#    echo ""            >> $CONFIGTMP
#    echo "TOP=@@TOP@@" >> $CONFIGTMP
#    echo "PERL5LIB=\$TOP/lib/perl5/\$VERSION"     >> $CONFIGTMP
#    echo "PREFIX=$PREFIX"                         >> $CONFIGTMP
#    echo "CONF=\$(\$TOP/bin/perl \$*)"            >> $CONFIGTMP
#    echo ""                                       >> $CONFIGTMP
#    echo "PREFIX=\$(echo \$PREFIX | sed -e 's/\//\\\\\//g')" >> $CONFIGTMP
#    echo "TOP=\$(echo \$TOP | sed -e 's/\//\\\\\//g')"     >> $CONFIGTMP
#    echo "echo \$CONF | sed -e 's/'\$PREFIX'/'\$TOP'/g'" >> $CONFIGTMP
#    chmod +x $CONFIGTMP

    BEFORE=$TODIR/$PREFIX/mipkg/before-PKG-INSTALL
#    echo "#! /bin/sh"                                 > $BEFORE
#    echo "echo \"Hei igjen\""                              >> $BEFORE
#    echo "TOP=\$(pwd)"                               >> $BEFORE
#    echo "TOP=\$(echo \$TOP | sed -e 's/\//\\\\\//g')" >> $BEFORE
#    echo "sed -e 's/@@TOP@@/'\$TOP'/' \$1/bin/kvperl-config.tmp > \$1/bin/kvperl-config" >> $BEFORE 
#   echo "chmod +x \$1/bin/kvperl-config" >> $BEFORE
    echo "#! /bin/sh"    > $BEFORE
    echo "cd \$1/bin || (echo \"Cant change to <\$1>!\"; exit 1)" >> $BEFORE 
    echo "ln -s perl kvperl" >> $BEFORE
    chmod +x $BEFORE

    echo "lib/perl5/$VERSION" >  $TODIR/$PREFIX/mipkg/files
    echo "bin/kvperl"           >> $TODIR/$PREFIX/mipkg/files
    #echo "bin/kvperl-config"  >> $TODIR/$PREFIX/mipkg/files
else
    echo "Which package shall be generated LIB or DEV!"
    use
fi

BASE=$(basename $TODIR/$PREFIX)
mv $TODIR/$PREFIX /$BASE/$NAME-$PKG-$VERSION

cd /$BASE || (echo "Cant change directory to <$BASE>!"; exit 1)

tar cpf $NAME-$PKG-$VERSION.tar $NAME-$PKG-$VERSION
gzip $NAME-$PKG-$VERSION.tar

mv $NAME-$PKG-$VERSION.tar.gz $TOP

exit 0
