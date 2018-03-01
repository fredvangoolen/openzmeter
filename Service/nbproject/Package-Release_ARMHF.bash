#!/bin/bash -x

#
# Generated - do not edit!
#

# Macros
TOP=`pwd`
CND_PLATFORM=GNU-Linux
CND_CONF=Release_ARMHF
CND_DISTDIR=dist
CND_BUILDDIR=build
CND_DLIB_EXT=so
NBTMPDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}/tmp-packaging
TMPDIRNAME=tmp-packaging
OUTPUT_PATH=${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/openZmeter
OUTPUT_BASENAME=openZmeter
PACKAGE_TOP_DIR=/opt/openZmeter/

# Functions
function checkReturnCode
{
    rc=$?
    if [ $rc != 0 ]
    then
        exit $rc
    fi
}
function makeDirectory
# $1 directory path
# $2 permission (optional)
{
    mkdir -p "$1"
    checkReturnCode
    if [ "$2" != "" ]
    then
      chmod $2 "$1"
      checkReturnCode
    fi
}
function copyFileToTmpDir
# $1 from-file path
# $2 to-file path
# $3 permission
{
    cp "$1" "$2"
    checkReturnCode
    if [ "$3" != "" ]
    then
        chmod $3 "$2"
        checkReturnCode
    fi
}

# Setup
cd "${TOP}"
mkdir -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/package
rm -rf ${NBTMPDIR}
mkdir -p ${NBTMPDIR}

# Copy files and create directories and links
cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB"
copyFileToTmpDir "WEB/Events_EN50160.html" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Events_EN50160.html" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB"
copyFileToTmpDir "WEB/Analysis_EnergyPower.html" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Analysis_EnergyPower.html" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB"
copyFileToTmpDir "WEB/Events_ITIC.html" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Events_ITIC.html" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB"
copyFileToTmpDir "WEB/Help.html" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Help.html" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources"
copyFileToTmpDir "WEB/Resources/Analysis_Frequency.js" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/Analysis_Frequency.js" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources"
copyFileToTmpDir "WEB/Resources/Route_1Phase_4.svg" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/Route_1Phase_4.svg" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/datatables/js"
copyFileToTmpDir "WEB/Resources/lib/datatables/js/dataTables.bootstrap.js" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/datatables/js/dataTables.bootstrap.js" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/datatables/js"
copyFileToTmpDir "WEB/Resources/lib/datatables/js/dataTables.responsive.min.js" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/datatables/js/dataTables.responsive.min.js" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/datatables/js"
copyFileToTmpDir "WEB/Resources/lib/datatables/js/jquery.dataTables.min.js" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/datatables/js/jquery.dataTables.min.js" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/datatables/images"
copyFileToTmpDir "WEB/Resources/lib/datatables/images/sort_desc_disabled.png" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/datatables/images/sort_desc_disabled.png" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/datatables/images"
copyFileToTmpDir "WEB/Resources/lib/datatables/images/sort_asc_disabled.png" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/datatables/images/sort_asc_disabled.png" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/datatables/images"
copyFileToTmpDir "WEB/Resources/lib/datatables/images/favicon.ico" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/datatables/images/favicon.ico" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/datatables/images"
copyFileToTmpDir "WEB/Resources/lib/datatables/images/sort_both.png" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/datatables/images/sort_both.png" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/datatables/images"
copyFileToTmpDir "WEB/Resources/lib/datatables/images/sort_asc.png" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/datatables/images/sort_asc.png" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/datatables/images"
copyFileToTmpDir "WEB/Resources/lib/datatables/images/sort_desc.png" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/datatables/images/sort_desc.png" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/datatables/css"
copyFileToTmpDir "WEB/Resources/lib/datatables/css/dataTables.bootstrap.css" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/datatables/css/dataTables.bootstrap.css" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/datatables/css"
copyFileToTmpDir "WEB/Resources/lib/datatables/css/jquery.dataTables.min.css" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/datatables/css/jquery.dataTables.min.css" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/datatables/css"
copyFileToTmpDir "WEB/Resources/lib/datatables/css/dataTables.responsive.css" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/datatables/css/dataTables.responsive.css" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/dc.js"
copyFileToTmpDir "WEB/Resources/lib/dc.js/dc.min.js" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/dc.js/dc.min.js" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/dc.js"
copyFileToTmpDir "WEB/Resources/lib/dc.js/dc.css" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/dc.js/dc.css" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/dc.js"
copyFileToTmpDir "WEB/Resources/lib/dc.js/crossfilter.js" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/dc.js/crossfilter.js" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/scrollto"
copyFileToTmpDir "WEB/Resources/lib/scrollto/jquery.scrollTo.min.js" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/scrollto/jquery.scrollTo.min.js" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/spin.js"
copyFileToTmpDir "WEB/Resources/lib/spin.js/spin.min.js" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/spin.js/spin.min.js" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/metisMenu"
copyFileToTmpDir "WEB/Resources/lib/metisMenu/metisMenu.min.css" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/metisMenu/metisMenu.min.css" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/metisMenu"
copyFileToTmpDir "WEB/Resources/lib/metisMenu/metisMenu.js" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/metisMenu/metisMenu.js" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/metisMenu"
copyFileToTmpDir "WEB/Resources/lib/metisMenu/.bower.json" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/metisMenu/.bower.json" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/metisMenu"
copyFileToTmpDir "WEB/Resources/lib/metisMenu/metisMenu.css" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/metisMenu/metisMenu.css" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/metisMenu"
copyFileToTmpDir "WEB/Resources/lib/metisMenu/metisMenu.min.js" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/metisMenu/metisMenu.min.js" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/nvd3"
copyFileToTmpDir "WEB/Resources/lib/nvd3/nv.d3.min.js" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/nvd3/nv.d3.min.js" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/nvd3"
copyFileToTmpDir "WEB/Resources/lib/nvd3/nv.d3.css" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/nvd3/nv.d3.css" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/bootstrap-validator/language"
copyFileToTmpDir "WEB/Resources/lib/bootstrap-validator/language/ja_JP.js" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/bootstrap-validator/language/ja_JP.js" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/bootstrap-validator/language"
copyFileToTmpDir "WEB/Resources/lib/bootstrap-validator/language/no_NO.js" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/bootstrap-validator/language/no_NO.js" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/bootstrap-validator/language"
copyFileToTmpDir "WEB/Resources/lib/bootstrap-validator/language/en_US.js" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/bootstrap-validator/language/en_US.js" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/bootstrap-validator/language"
copyFileToTmpDir "WEB/Resources/lib/bootstrap-validator/language/sv_SE.js" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/bootstrap-validator/language/sv_SE.js" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/bootstrap-validator/language"
copyFileToTmpDir "WEB/Resources/lib/bootstrap-validator/language/zh_CN.js" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/bootstrap-validator/language/zh_CN.js" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/bootstrap-validator/language"
copyFileToTmpDir "WEB/Resources/lib/bootstrap-validator/language/th_TH.js" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/bootstrap-validator/language/th_TH.js" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/bootstrap-validator/language"
copyFileToTmpDir "WEB/Resources/lib/bootstrap-validator/language/ua_UA.js" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/bootstrap-validator/language/ua_UA.js" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/bootstrap-validator/language"
copyFileToTmpDir "WEB/Resources/lib/bootstrap-validator/language/ro_RO.js" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/bootstrap-validator/language/ro_RO.js" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/bootstrap-validator/language"
copyFileToTmpDir "WEB/Resources/lib/bootstrap-validator/language/id_ID.js" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/bootstrap-validator/language/id_ID.js" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/bootstrap-validator/language"
copyFileToTmpDir "WEB/Resources/lib/bootstrap-validator/language/da_DK.js" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/bootstrap-validator/language/da_DK.js" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/bootstrap-validator/language"
copyFileToTmpDir "WEB/Resources/lib/bootstrap-validator/language/vi_VN.js" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/bootstrap-validator/language/vi_VN.js" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/bootstrap-validator/language"
copyFileToTmpDir "WEB/Resources/lib/bootstrap-validator/language/hu_HU.js" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/bootstrap-validator/language/hu_HU.js" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/bootstrap-validator/language"
copyFileToTmpDir "WEB/Resources/lib/bootstrap-validator/language/be_FR.js" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/bootstrap-validator/language/be_FR.js" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/bootstrap-validator/language"
copyFileToTmpDir "WEB/Resources/lib/bootstrap-validator/language/ru_RU.js" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/bootstrap-validator/language/ru_RU.js" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/bootstrap-validator/language"
copyFileToTmpDir "WEB/Resources/lib/bootstrap-validator/language/zh_TW.js" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/bootstrap-validator/language/zh_TW.js" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/bootstrap-validator/language"
copyFileToTmpDir "WEB/Resources/lib/bootstrap-validator/language/de_DE.js" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/bootstrap-validator/language/de_DE.js" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/bootstrap-validator/language"
copyFileToTmpDir "WEB/Resources/lib/bootstrap-validator/language/be_NL.js" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/bootstrap-validator/language/be_NL.js" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/bootstrap-validator/language"
copyFileToTmpDir "WEB/Resources/lib/bootstrap-validator/language/es_ES.js" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/bootstrap-validator/language/es_ES.js" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/bootstrap-validator/language"
copyFileToTmpDir "WEB/Resources/lib/bootstrap-validator/language/nl_NL.js" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/bootstrap-validator/language/nl_NL.js" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/bootstrap-validator/language"
copyFileToTmpDir "WEB/Resources/lib/bootstrap-validator/language/gr_EL.js" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/bootstrap-validator/language/gr_EL.js" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/bootstrap-validator/language"
copyFileToTmpDir "WEB/Resources/lib/bootstrap-validator/language/es_CL.js" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/bootstrap-validator/language/es_CL.js" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/bootstrap-validator/language"
copyFileToTmpDir "WEB/Resources/lib/bootstrap-validator/language/sr_RS.js" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/bootstrap-validator/language/sr_RS.js" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/bootstrap-validator/language"
copyFileToTmpDir "WEB/Resources/lib/bootstrap-validator/language/tr_TR.js" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/bootstrap-validator/language/tr_TR.js" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/bootstrap-validator/language"
copyFileToTmpDir "WEB/Resources/lib/bootstrap-validator/language/fr_FR.js" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/bootstrap-validator/language/fr_FR.js" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/bootstrap-validator/language"
copyFileToTmpDir "WEB/Resources/lib/bootstrap-validator/language/pl_PL.js" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/bootstrap-validator/language/pl_PL.js" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/bootstrap-validator/language"
copyFileToTmpDir "WEB/Resources/lib/bootstrap-validator/language/it_IT.js" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/bootstrap-validator/language/it_IT.js" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/bootstrap-validator/language"
copyFileToTmpDir "WEB/Resources/lib/bootstrap-validator/language/pt_BR.js" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/bootstrap-validator/language/pt_BR.js" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/bootstrap-validator/language"
copyFileToTmpDir "WEB/Resources/lib/bootstrap-validator/language/bg_BG.js" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/bootstrap-validator/language/bg_BG.js" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/bootstrap-validator/language"
copyFileToTmpDir "WEB/Resources/lib/bootstrap-validator/language/cs_CZ.js" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/bootstrap-validator/language/cs_CZ.js" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/bootstrap-validator/language"
copyFileToTmpDir "WEB/Resources/lib/bootstrap-validator/language/ar_MA.js" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/bootstrap-validator/language/ar_MA.js" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/bootstrap-validator/language"
copyFileToTmpDir "WEB/Resources/lib/bootstrap-validator/language/fa_IR.js" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/bootstrap-validator/language/fa_IR.js" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/bootstrap-validator"
copyFileToTmpDir "WEB/Resources/lib/bootstrap-validator/bootstrapValidator.min.css" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/bootstrap-validator/bootstrapValidator.min.css" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/bootstrap-validator"
copyFileToTmpDir "WEB/Resources/lib/bootstrap-validator/bootstrapValidator.min.js" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/bootstrap-validator/bootstrapValidator.min.js" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/qtip2"
copyFileToTmpDir "WEB/Resources/lib/qtip2/jquery.qtip.min.js" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/qtip2/jquery.qtip.min.js" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/qtip2"
copyFileToTmpDir "WEB/Resources/lib/qtip2/jquery.qtip.js" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/qtip2/jquery.qtip.js" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/qtip2"
copyFileToTmpDir "WEB/Resources/lib/qtip2/jquery.qtip.css" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/qtip2/jquery.qtip.css" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/qtip2"
copyFileToTmpDir "WEB/Resources/lib/qtip2/imagesloaded.pkg.min.js" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/qtip2/imagesloaded.pkg.min.js" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/qtip2"
copyFileToTmpDir "WEB/Resources/lib/qtip2/imagesloaded.pkg.min.js.map" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/qtip2/imagesloaded.pkg.min.js.map" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/qtip2"
copyFileToTmpDir "WEB/Resources/lib/qtip2/jquery.qtip.min.js.map" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/qtip2/jquery.qtip.min.js.map" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/qtip2"
copyFileToTmpDir "WEB/Resources/lib/qtip2/jquery.qtip.min.css" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/qtip2/jquery.qtip.min.css" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/font-awesome"
copyFileToTmpDir "WEB/Resources/lib/font-awesome/font-awesome.min.css" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/font-awesome/font-awesome.min.css" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/font-awesome/fonts"
copyFileToTmpDir "WEB/Resources/lib/font-awesome/fonts/fontawesome-webfont.ttf" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/font-awesome/fonts/fontawesome-webfont.ttf" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/font-awesome/fonts"
copyFileToTmpDir "WEB/Resources/lib/font-awesome/fonts/FontAwesome.otf" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/font-awesome/fonts/FontAwesome.otf" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/font-awesome/fonts"
copyFileToTmpDir "WEB/Resources/lib/font-awesome/fonts/fontawesome-webfont.svg" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/font-awesome/fonts/fontawesome-webfont.svg" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/font-awesome/fonts"
copyFileToTmpDir "WEB/Resources/lib/font-awesome/fonts/fontawesome-webfont.woff" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/font-awesome/fonts/fontawesome-webfont.woff" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/font-awesome/fonts"
copyFileToTmpDir "WEB/Resources/lib/font-awesome/fonts/fontawesome-webfont.woff2" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/font-awesome/fonts/fontawesome-webfont.woff2" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/font-awesome/fonts"
copyFileToTmpDir "WEB/Resources/lib/font-awesome/fonts/fontawesome-webfont.eot" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/font-awesome/fonts/fontawesome-webfont.eot" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/font-awesome/css"
copyFileToTmpDir "WEB/Resources/lib/font-awesome/css/font-awesome.min.css" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/font-awesome/css/font-awesome.min.css" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib"
copyFileToTmpDir "WEB/Resources/lib/jquery.min.js" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/jquery.min.js" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/jquery.lineProgressbar"
copyFileToTmpDir "WEB/Resources/lib/jquery.lineProgressbar/jquery.lineProgressbar.js" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/jquery.lineProgressbar/jquery.lineProgressbar.js" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/jquery.lineProgressbar"
copyFileToTmpDir "WEB/Resources/lib/jquery.lineProgressbar/jquery.lineProgressbar.css" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/jquery.lineProgressbar/jquery.lineProgressbar.css" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/c3js"
copyFileToTmpDir "WEB/Resources/lib/c3js/c3.min.js" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/c3js/c3.min.js" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/c3js"
copyFileToTmpDir "WEB/Resources/lib/c3js/c3.css" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/c3js/c3.css" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/bootstrap/js"
copyFileToTmpDir "WEB/Resources/lib/bootstrap/js/bootstrap.min.js" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/bootstrap/js/bootstrap.min.js" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/bootstrap/fonts"
copyFileToTmpDir "WEB/Resources/lib/bootstrap/fonts/glyphicons-halflings-regular.woff" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/bootstrap/fonts/glyphicons-halflings-regular.woff" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/bootstrap/fonts"
copyFileToTmpDir "WEB/Resources/lib/bootstrap/fonts/glyphicons-halflings-regular.eot" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/bootstrap/fonts/glyphicons-halflings-regular.eot" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/bootstrap/fonts"
copyFileToTmpDir "WEB/Resources/lib/bootstrap/fonts/glyphicons-halflings-regular.ttf" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/bootstrap/fonts/glyphicons-halflings-regular.ttf" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/bootstrap/fonts"
copyFileToTmpDir "WEB/Resources/lib/bootstrap/fonts/glyphicons-halflings-regular.svg" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/bootstrap/fonts/glyphicons-halflings-regular.svg" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/bootstrap/css"
copyFileToTmpDir "WEB/Resources/lib/bootstrap/css/bootstrap-theme.css" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/bootstrap/css/bootstrap-theme.css" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/bootstrap/css"
copyFileToTmpDir "WEB/Resources/lib/bootstrap/css/bootstrap.min.css" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/bootstrap/css/bootstrap.min.css" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/bootstrap/css"
copyFileToTmpDir "WEB/Resources/lib/bootstrap/css/bootstrap.css.map" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/bootstrap/css/bootstrap.css.map" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/bootstrap/css"
copyFileToTmpDir "WEB/Resources/lib/bootstrap/css/bootstrap-theme.css.map" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/bootstrap/css/bootstrap-theme.css.map" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/bootstrap/css"
copyFileToTmpDir "WEB/Resources/lib/bootstrap/css/bootstrap-theme.min.css" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/bootstrap/css/bootstrap-theme.min.css" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/bootstrap/css"
copyFileToTmpDir "WEB/Resources/lib/bootstrap/css/bootstrap.css" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/bootstrap/css/bootstrap.css" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/jquery-ui/js"
copyFileToTmpDir "WEB/Resources/lib/jquery-ui/js/jquery-ui.min.js" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/jquery-ui/js/jquery-ui.min.js" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/jquery-ui/css/smoothness/images"
copyFileToTmpDir "WEB/Resources/lib/jquery-ui/css/smoothness/images/ui-bg_flat_0_aaaaaa_40x100.png" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/jquery-ui/css/smoothness/images/ui-bg_flat_0_aaaaaa_40x100.png" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/jquery-ui/css/smoothness/images"
copyFileToTmpDir "WEB/Resources/lib/jquery-ui/css/smoothness/images/ui-bg_glass_65_ffffff_1x400.png" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/jquery-ui/css/smoothness/images/ui-bg_glass_65_ffffff_1x400.png" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/jquery-ui/css/smoothness/images"
copyFileToTmpDir "WEB/Resources/lib/jquery-ui/css/smoothness/images/ui-bg_highlight-soft_75_cccccc_1x100.png" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/jquery-ui/css/smoothness/images/ui-bg_highlight-soft_75_cccccc_1x100.png" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/jquery-ui/css/smoothness/images"
copyFileToTmpDir "WEB/Resources/lib/jquery-ui/css/smoothness/images/ui-bg_glass_95_fef1ec_1x400.png" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/jquery-ui/css/smoothness/images/ui-bg_glass_95_fef1ec_1x400.png" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/jquery-ui/css/smoothness/images"
copyFileToTmpDir "WEB/Resources/lib/jquery-ui/css/smoothness/images/ui-icons_2e83ff_256x240.png" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/jquery-ui/css/smoothness/images/ui-icons_2e83ff_256x240.png" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/jquery-ui/css/smoothness/images"
copyFileToTmpDir "WEB/Resources/lib/jquery-ui/css/smoothness/images/ui-bg_glass_75_e6e6e6_1x400.png" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/jquery-ui/css/smoothness/images/ui-bg_glass_75_e6e6e6_1x400.png" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/jquery-ui/css/smoothness/images"
copyFileToTmpDir "WEB/Resources/lib/jquery-ui/css/smoothness/images/ui-icons_222222_256x240.png" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/jquery-ui/css/smoothness/images/ui-icons_222222_256x240.png" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/jquery-ui/css/smoothness/images"
copyFileToTmpDir "WEB/Resources/lib/jquery-ui/css/smoothness/images/ui-icons_454545_256x240.png" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/jquery-ui/css/smoothness/images/ui-icons_454545_256x240.png" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/jquery-ui/css/smoothness/images"
copyFileToTmpDir "WEB/Resources/lib/jquery-ui/css/smoothness/images/ui-bg_flat_75_ffffff_40x100.png" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/jquery-ui/css/smoothness/images/ui-bg_flat_75_ffffff_40x100.png" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/jquery-ui/css/smoothness/images"
copyFileToTmpDir "WEB/Resources/lib/jquery-ui/css/smoothness/images/ui-bg_glass_55_fbf9ee_1x400.png" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/jquery-ui/css/smoothness/images/ui-bg_glass_55_fbf9ee_1x400.png" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/jquery-ui/css/smoothness/images"
copyFileToTmpDir "WEB/Resources/lib/jquery-ui/css/smoothness/images/ui-icons_888888_256x240.png" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/jquery-ui/css/smoothness/images/ui-icons_888888_256x240.png" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/jquery-ui/css/smoothness/images"
copyFileToTmpDir "WEB/Resources/lib/jquery-ui/css/smoothness/images/ui-bg_glass_75_dadada_1x400.png" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/jquery-ui/css/smoothness/images/ui-bg_glass_75_dadada_1x400.png" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/jquery-ui/css/smoothness/images"
copyFileToTmpDir "WEB/Resources/lib/jquery-ui/css/smoothness/images/ui-icons_cd0a0a_256x240.png" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/jquery-ui/css/smoothness/images/ui-icons_cd0a0a_256x240.png" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/jquery-ui/css/smoothness"
copyFileToTmpDir "WEB/Resources/lib/jquery-ui/css/smoothness/jquery-ui-1.8.10.custom.css" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/jquery-ui/css/smoothness/jquery-ui-1.8.10.custom.css" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/bootstrap-year-calendar"
copyFileToTmpDir "WEB/Resources/lib/bootstrap-year-calendar/bootstrap-year-calendar.min.css" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/bootstrap-year-calendar/bootstrap-year-calendar.min.css" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/bootstrap-year-calendar"
copyFileToTmpDir "WEB/Resources/lib/bootstrap-year-calendar/bootstrap-year-calendar.min.js" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/bootstrap-year-calendar/bootstrap-year-calendar.min.js" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/jQRangeSlider"
copyFileToTmpDir "WEB/Resources/lib/jQRangeSlider/jQDateRangeSlider.css" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/jQRangeSlider/jQDateRangeSlider.css" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/jQRangeSlider"
copyFileToTmpDir "WEB/Resources/lib/jQRangeSlider/jquery.mousewheel.min.js" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/jQRangeSlider/jquery.mousewheel.min.js" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/jQRangeSlider"
copyFileToTmpDir "WEB/Resources/lib/jQRangeSlider/jQDateRangeSlider-withRuler-min.js" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/jQRangeSlider/jQDateRangeSlider-withRuler-min.js" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib"
copyFileToTmpDir "WEB/Resources/lib/chroma.min.js" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/chroma.min.js" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/offline"
copyFileToTmpDir "WEB/Resources/lib/offline/offline.css" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/offline/offline.css" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/offline"
copyFileToTmpDir "WEB/Resources/lib/offline/offline.min.js" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/offline/offline.min.js" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/d3-tip"
copyFileToTmpDir "WEB/Resources/lib/d3-tip/d3-tip.min.css" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/d3-tip/d3-tip.min.css" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/d3-tip"
copyFileToTmpDir "WEB/Resources/lib/d3-tip/d3-tip.min.js" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/d3-tip/d3-tip.min.js" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/dygraph"
copyFileToTmpDir "WEB/Resources/lib/dygraph/chart-plotter.js" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/dygraph/chart-plotter.js" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/dygraph"
copyFileToTmpDir "WEB/Resources/lib/dygraph/dygraph.js" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/dygraph/dygraph.js" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/dygraph"
copyFileToTmpDir "WEB/Resources/lib/dygraph/crosshair.js" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/dygraph/crosshair.js" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/dygraph"
copyFileToTmpDir "WEB/Resources/lib/dygraph/dygraph.css" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/dygraph/dygraph.css" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/dygraph"
copyFileToTmpDir "WEB/Resources/lib/dygraph/synchronize.js" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/dygraph/synchronize.js" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/dygraph"
copyFileToTmpDir "WEB/Resources/lib/dygraph/dygraph-interaction.js" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/dygraph/dygraph-interaction.js" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/dygraph"
copyFileToTmpDir "WEB/Resources/lib/dygraph/dygraph.min.js" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/dygraph/dygraph.min.js" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/momentjs"
copyFileToTmpDir "WEB/Resources/lib/momentjs/moment.min.js" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/momentjs/moment.min.js" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/momentjs"
copyFileToTmpDir "WEB/Resources/lib/momentjs/moment-duration-format.js" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/momentjs/moment-duration-format.js" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/livestamp"
copyFileToTmpDir "WEB/Resources/lib/livestamp/livestamp.min.js" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/livestamp/livestamp.min.js" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib"
copyFileToTmpDir "WEB/Resources/lib/marked.min.js" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/marked.min.js" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/d3"
copyFileToTmpDir "WEB/Resources/lib/d3/colorbrewer.css" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/d3/colorbrewer.css" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/d3"
copyFileToTmpDir "WEB/Resources/lib/d3/colorbrewer.js" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/d3/colorbrewer.js" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/d3"
copyFileToTmpDir "WEB/Resources/lib/d3/d3.v3.min_bak.js" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/d3/d3.v3.min_bak.js" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/d3"
copyFileToTmpDir "WEB/Resources/lib/d3/d3.v3.min.js" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/d3/d3.v3.min.js" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/visjs"
copyFileToTmpDir "WEB/Resources/lib/visjs/vis.min.js" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/visjs/vis.min.js" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/visjs"
copyFileToTmpDir "WEB/Resources/lib/visjs/vis.min.css" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/visjs/vis.min.css" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/visjs/img/network"
copyFileToTmpDir "WEB/Resources/lib/visjs/img/network/zoomExtends.png" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/visjs/img/network/zoomExtends.png" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/visjs/img/network"
copyFileToTmpDir "WEB/Resources/lib/visjs/img/network/cross.png" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/visjs/img/network/cross.png" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/visjs/img/network"
copyFileToTmpDir "WEB/Resources/lib/visjs/img/network/upArrow.png" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/visjs/img/network/upArrow.png" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/visjs/img/network"
copyFileToTmpDir "WEB/Resources/lib/visjs/img/network/editIcon.png" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/visjs/img/network/editIcon.png" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/visjs/img/network"
copyFileToTmpDir "WEB/Resources/lib/visjs/img/network/deleteIcon.png" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/visjs/img/network/deleteIcon.png" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/visjs/img/network"
copyFileToTmpDir "WEB/Resources/lib/visjs/img/network/leftArrow.png" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/visjs/img/network/leftArrow.png" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/visjs/img/network"
copyFileToTmpDir "WEB/Resources/lib/visjs/img/network/rightArrow.png" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/visjs/img/network/rightArrow.png" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/visjs/img/network"
copyFileToTmpDir "WEB/Resources/lib/visjs/img/network/minus.png" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/visjs/img/network/minus.png" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/visjs/img/network"
copyFileToTmpDir "WEB/Resources/lib/visjs/img/network/backIcon.png" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/visjs/img/network/backIcon.png" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/visjs/img/network"
copyFileToTmpDir "WEB/Resources/lib/visjs/img/network/addNodeIcon.png" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/visjs/img/network/addNodeIcon.png" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/visjs/img/network"
copyFileToTmpDir "WEB/Resources/lib/visjs/img/network/acceptDeleteIcon.png" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/visjs/img/network/acceptDeleteIcon.png" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/visjs/img/network"
copyFileToTmpDir "WEB/Resources/lib/visjs/img/network/downArrow.png" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/visjs/img/network/downArrow.png" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/visjs/img/network"
copyFileToTmpDir "WEB/Resources/lib/visjs/img/network/plus.png" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/visjs/img/network/plus.png" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/visjs/img/network"
copyFileToTmpDir "WEB/Resources/lib/visjs/img/network/connectIcon.png" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/visjs/img/network/connectIcon.png" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/visjs/img/network"
copyFileToTmpDir "WEB/Resources/lib/visjs/img/network/cross2.png" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/visjs/img/network/cross2.png" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/lib/visjs/img/timeline"
copyFileToTmpDir "WEB/Resources/lib/visjs/img/timeline/delete.png" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/lib/visjs/img/timeline/delete.png" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources"
copyFileToTmpDir "WEB/Resources/Events_Disturbances.js" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/Events_Disturbances.js" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources"
copyFileToTmpDir "WEB/Resources/Dashboard.js" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/Dashboard.js" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources"
copyFileToTmpDir "WEB/Resources/Analysis_Current.js" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/Analysis_Current.js" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources"
copyFileToTmpDir "WEB/Resources/Help.js" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/Help.js" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/img"
copyFileToTmpDir "WEB/Resources/img/loading.gif" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/img/loading.gif" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/img"
copyFileToTmpDir "WEB/Resources/img/logo.png" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/img/logo.png" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/img"
copyFileToTmpDir "WEB/Resources/img/load.gif" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/img/load.gif" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources"
copyFileToTmpDir "WEB/Resources/Analysis_EnergyPower.js" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/Analysis_EnergyPower.js" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources"
copyFileToTmpDir "WEB/Resources/Stats.js" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/Stats.js" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/js"
copyFileToTmpDir "WEB/Resources/js/live.js" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/js/live.js" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/js"
copyFileToTmpDir "WEB/Resources/js/utils.js" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/js/utils.js" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/js"
copyFileToTmpDir "WEB/Resources/js/settings.js" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/js/settings.js" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/js"
copyFileToTmpDir "WEB/Resources/js/zTicker.js" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/js/zTicker.js" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/js/events"
copyFileToTmpDir "WEB/Resources/js/events/events.js" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/js/events/events.js" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/js"
copyFileToTmpDir "WEB/Resources/js/account.js" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/js/account.js" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources"
copyFileToTmpDir "WEB/Resources/Route_1Phase_2.svg" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/Route_1Phase_2.svg" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources"
copyFileToTmpDir "WEB/Resources/Analysis_Voltage.js" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/Analysis_Voltage.js" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources"
copyFileToTmpDir "WEB/Resources/CommonHeader.html" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/CommonHeader.html" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources"
copyFileToTmpDir "WEB/Resources/Route_3Phase.svg" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/Route_3Phase.svg" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources"
copyFileToTmpDir "WEB/Resources/openZmeter.js" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/openZmeter.js" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources"
copyFileToTmpDir "WEB/Resources/Login.js" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/Login.js" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources"
copyFileToTmpDir "WEB/Resources/Events_EN50160.js" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/Events_EN50160.js" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources"
copyFileToTmpDir "WEB/Resources/favicon.ico" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/favicon.ico" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources"
copyFileToTmpDir "WEB/Resources/Route_1Phase_1.svg" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/Route_1Phase_1.svg" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/HelpPages"
copyFileToTmpDir "WEB/Resources/HelpPages/3f.png" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/HelpPages/3f.png" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/HelpPages"
copyFileToTmpDir "WEB/Resources/HelpPages/ozm.png" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/HelpPages/ozm.png" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/HelpPages"
copyFileToTmpDir "WEB/Resources/HelpPages/Configuración.md" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/HelpPages/Configuración.md" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/HelpPages"
copyFileToTmpDir "WEB/Resources/HelpPages/oZmlink.md" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/HelpPages/oZmlink.md" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/HelpPages"
copyFileToTmpDir "WEB/Resources/HelpPages/Variables medidas.md" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/HelpPages/Variables medidas.md" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/HelpPages"
copyFileToTmpDir "WEB/Resources/HelpPages/fundamental_reactive_power.png" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/HelpPages/fundamental_reactive_power.png" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/HelpPages"
copyFileToTmpDir "WEB/Resources/HelpPages/rms_i.png" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/HelpPages/rms_i.png" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/HelpPages"
copyFileToTmpDir "WEB/Resources/HelpPages/thd_i.png" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/HelpPages/thd_i.png" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/HelpPages"
copyFileToTmpDir "WEB/Resources/HelpPages/API.md" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/HelpPages/API.md" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/HelpPages"
copyFileToTmpDir "WEB/Resources/HelpPages/Hardware (English).md" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/HelpPages/Hardware (English).md" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/HelpPages"
copyFileToTmpDir "WEB/Resources/HelpPages/Zero Crossing.md" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/HelpPages/Zero Crossing.md" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/HelpPages"
copyFileToTmpDir "WEB/Resources/HelpPages/CodeCogsEqn.png" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/HelpPages/CodeCogsEqn.png" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/HelpPages"
copyFileToTmpDir "WEB/Resources/HelpPages/Captura de pantalla 2017-12-10 a las 18.18.02.png" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/HelpPages/Captura de pantalla 2017-12-10 a las 18.18.02.png" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/HelpPages"
copyFileToTmpDir "WEB/Resources/HelpPages/fundamental_apparent_power.png" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/HelpPages/fundamental_apparent_power.png" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/HelpPages"
copyFileToTmpDir "WEB/Resources/HelpPages/Hardware.md" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/HelpPages/Hardware.md" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/HelpPages"
copyFileToTmpDir "WEB/Resources/HelpPages/active_power.png.png" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/HelpPages/active_power.png.png" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/HelpPages"
copyFileToTmpDir "WEB/Resources/HelpPages/apparent_power.png" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/HelpPages/apparent_power.png" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/HelpPages"
copyFileToTmpDir "WEB/Resources/HelpPages/reactive_power.png" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/HelpPages/reactive_power.png" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/HelpPages"
copyFileToTmpDir "WEB/Resources/HelpPages/power_system_freq_meas.pdf" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/HelpPages/power_system_freq_meas.pdf" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/HelpPages"
copyFileToTmpDir "WEB/Resources/HelpPages/CodeCogsEqn (1).png" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/HelpPages/CodeCogsEqn (1).png" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/HelpPages"
copyFileToTmpDir "WEB/Resources/HelpPages/1F.png" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/HelpPages/1F.png" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/HelpPages"
copyFileToTmpDir "WEB/Resources/HelpPages/active_power.png" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/HelpPages/active_power.png" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/HelpPages"
copyFileToTmpDir "WEB/Resources/HelpPages/fundamental_active_power.png" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/HelpPages/fundamental_active_power.png" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/HelpPages"
copyFileToTmpDir "WEB/Resources/HelpPages/Estandar EN 50160.md" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/HelpPages/Estandar EN 50160.md" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/HelpPages"
copyFileToTmpDir "WEB/Resources/HelpPages/311-interharmonics.pdf" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/HelpPages/311-interharmonics.pdf" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/HelpPages"
copyFileToTmpDir "WEB/Resources/HelpPages/Standard IEC61000-4-30.md" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/HelpPages/Standard IEC61000-4-30.md" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/HelpPages"
copyFileToTmpDir "WEB/Resources/HelpPages/3F.png" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/HelpPages/3F.png" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/HelpPages"
copyFileToTmpDir "WEB/Resources/HelpPages/zeroC_interharmonic.png" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/HelpPages/zeroC_interharmonic.png" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/HelpPages"
copyFileToTmpDir "WEB/Resources/HelpPages/Home (English).md" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/HelpPages/Home (English).md" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/HelpPages"
copyFileToTmpDir "WEB/Resources/HelpPages/Measured variables.md" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/HelpPages/Measured variables.md" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/HelpPages"
copyFileToTmpDir "WEB/Resources/HelpPages/rms_v.png" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/HelpPages/rms_v.png" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/HelpPages"
copyFileToTmpDir "WEB/Resources/HelpPages/Home.md" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/HelpPages/Home.md" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/HelpPages"
copyFileToTmpDir "WEB/Resources/HelpPages/Desbalanceo y potencia en sistemas trifasicos.pdf" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/HelpPages/Desbalanceo y potencia en sistemas trifásicos.pdf" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/HelpPages"
copyFileToTmpDir "WEB/Resources/HelpPages/4cuadrantes.png" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/HelpPages/4cuadrantes.png" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/HelpPages"
copyFileToTmpDir "WEB/Resources/HelpPages/thd_v.png" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/HelpPages/thd_v.png" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/HelpPages"
copyFileToTmpDir "WEB/Resources/HelpPages/Captura de pantalla 2017-11-25 a las 13.36.41.png" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/HelpPages/Captura de pantalla 2017-11-25 a las 13.36.41.png" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/HelpPages"
copyFileToTmpDir "WEB/Resources/HelpPages/zeroC.png" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/HelpPages/zeroC.png" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/HelpPages"
copyFileToTmpDir "WEB/Resources/HelpPages/4cuadrantes.jpg" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/HelpPages/4cuadrantes.jpg" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/HelpPages"
copyFileToTmpDir "WEB/Resources/HelpPages/Interharmonics- basic concepts and techniques for their detection and measurement  .pdf" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/HelpPages/Interharmonics- basic concepts and techniques for their detection and measurement  .pdf" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/HelpPages/.git/objects/pack"
copyFileToTmpDir "WEB/Resources/HelpPages/.git/objects/pack/pack-6f25feb59ab279e75710a75546fcc009124b1673.idx" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/HelpPages/.git/objects/pack/pack-6f25feb59ab279e75710a75546fcc009124b1673.idx" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/HelpPages/.git/objects/pack"
copyFileToTmpDir "WEB/Resources/HelpPages/.git/objects/pack/pack-6f25feb59ab279e75710a75546fcc009124b1673.pack" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/HelpPages/.git/objects/pack/pack-6f25feb59ab279e75710a75546fcc009124b1673.pack" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/HelpPages/.git/logs/refs/remotes/origin"
copyFileToTmpDir "WEB/Resources/HelpPages/.git/logs/refs/remotes/origin/HEAD" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/HelpPages/.git/logs/refs/remotes/origin/HEAD" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/HelpPages/.git/logs/refs/heads"
copyFileToTmpDir "WEB/Resources/HelpPages/.git/logs/refs/heads/master" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/HelpPages/.git/logs/refs/heads/master" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/HelpPages/.git/logs"
copyFileToTmpDir "WEB/Resources/HelpPages/.git/logs/HEAD" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/HelpPages/.git/logs/HEAD" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/HelpPages/.git"
copyFileToTmpDir "WEB/Resources/HelpPages/.git/description" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/HelpPages/.git/description" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/HelpPages/.git/refs/remotes/origin"
copyFileToTmpDir "WEB/Resources/HelpPages/.git/refs/remotes/origin/HEAD" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/HelpPages/.git/refs/remotes/origin/HEAD" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/HelpPages/.git/refs/heads"
copyFileToTmpDir "WEB/Resources/HelpPages/.git/refs/heads/master" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/HelpPages/.git/refs/heads/master" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/HelpPages/.git"
copyFileToTmpDir "WEB/Resources/HelpPages/.git/config" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/HelpPages/.git/config" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/HelpPages/.git"
copyFileToTmpDir "WEB/Resources/HelpPages/.git/packed-refs" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/HelpPages/.git/packed-refs" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/HelpPages/.git/info"
copyFileToTmpDir "WEB/Resources/HelpPages/.git/info/exclude" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/HelpPages/.git/info/exclude" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/HelpPages/.git"
copyFileToTmpDir "WEB/Resources/HelpPages/.git/index" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/HelpPages/.git/index" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/HelpPages/.git/hooks"
copyFileToTmpDir "WEB/Resources/HelpPages/.git/hooks/pre-receive.sample" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/HelpPages/.git/hooks/pre-receive.sample" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/HelpPages/.git/hooks"
copyFileToTmpDir "WEB/Resources/HelpPages/.git/hooks/pre-commit.sample" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/HelpPages/.git/hooks/pre-commit.sample" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/HelpPages/.git/hooks"
copyFileToTmpDir "WEB/Resources/HelpPages/.git/hooks/pre-push.sample" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/HelpPages/.git/hooks/pre-push.sample" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/HelpPages/.git/hooks"
copyFileToTmpDir "WEB/Resources/HelpPages/.git/hooks/pre-rebase.sample" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/HelpPages/.git/hooks/pre-rebase.sample" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/HelpPages/.git/hooks"
copyFileToTmpDir "WEB/Resources/HelpPages/.git/hooks/update.sample" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/HelpPages/.git/hooks/update.sample" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/HelpPages/.git/hooks"
copyFileToTmpDir "WEB/Resources/HelpPages/.git/hooks/applypatch-msg.sample" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/HelpPages/.git/hooks/applypatch-msg.sample" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/HelpPages/.git/hooks"
copyFileToTmpDir "WEB/Resources/HelpPages/.git/hooks/commit-msg.sample" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/HelpPages/.git/hooks/commit-msg.sample" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/HelpPages/.git/hooks"
copyFileToTmpDir "WEB/Resources/HelpPages/.git/hooks/pre-applypatch.sample" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/HelpPages/.git/hooks/pre-applypatch.sample" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/HelpPages/.git/hooks"
copyFileToTmpDir "WEB/Resources/HelpPages/.git/hooks/post-update.sample" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/HelpPages/.git/hooks/post-update.sample" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/HelpPages/.git/hooks"
copyFileToTmpDir "WEB/Resources/HelpPages/.git/hooks/prepare-commit-msg.sample" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/HelpPages/.git/hooks/prepare-commit-msg.sample" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/HelpPages/.git"
copyFileToTmpDir "WEB/Resources/HelpPages/.git/HEAD" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/HelpPages/.git/HEAD" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/HelpPages"
copyFileToTmpDir "WEB/Resources/HelpPages/Configuration.md" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/HelpPages/Configuration.md" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/HelpPages"
copyFileToTmpDir "WEB/Resources/HelpPages/Estandar IEC61000-4-30.md" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/HelpPages/Estandar IEC61000-4-30.md" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/css"
copyFileToTmpDir "WEB/Resources/css/common.css" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/css/common.css" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources/css"
copyFileToTmpDir "WEB/Resources/css/login.css" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/css/login.css" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources"
copyFileToTmpDir "WEB/Resources/Events_ITIC.js" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/Events_ITIC.js" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/Resources"
copyFileToTmpDir "WEB/Resources/Route_1Phase_3.svg" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Resources/Route_1Phase_3.svg" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB"
copyFileToTmpDir "WEB/Events_Disturbances.html" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Events_Disturbances.html" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB"
copyFileToTmpDir "WEB/Stats.html" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Stats.html" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB"
copyFileToTmpDir "WEB/Settings.html" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Settings.html" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB"
copyFileToTmpDir "WEB/Login.html" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Login.html" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB"
copyFileToTmpDir "WEB/Analysis_Frequency.html" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Analysis_Frequency.html" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB"
copyFileToTmpDir "WEB/Analysis_Voltage.html" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Analysis_Voltage.html" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB"
copyFileToTmpDir "WEB/Analysis_Current.html" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Analysis_Current.html" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB"
copyFileToTmpDir "WEB/Dashboard.html" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/Dashboard.html" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/MiniViews"
copyFileToTmpDir "WEB/MiniViews/Fundamental_Reactive_Power.html" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/MiniViews/Fundamental_Reactive_Power.html" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/MiniViews"
copyFileToTmpDir "WEB/MiniViews/Active_Energy.html" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/MiniViews/Active_Energy.html" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/MiniViews"
copyFileToTmpDir "WEB/MiniViews/Fundamental_Reactive_Energy.html" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/MiniViews/Fundamental_Reactive_Energy.html" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/MiniViews"
copyFileToTmpDir "WEB/MiniViews/Voltage.html" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/MiniViews/Voltage.html" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/MiniViews"
copyFileToTmpDir "WEB/MiniViews/Frequency.html" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/MiniViews/Frequency.html" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/MiniViews"
copyFileToTmpDir "WEB/MiniViews/Current.html" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/MiniViews/Current.html" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/WEB/MiniViews"
copyFileToTmpDir "WEB/MiniViews/Active_Power.html" "${NBTMPDIR}/${PACKAGE_TOP_DIR}WEB/MiniViews/Active_Power.html" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//etc"
copyFileToTmpDir "openZmeter.ini" "${NBTMPDIR}//etc/openZmeter.ini" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//DEBIAN"
copyFileToTmpDir "postinst" "${NBTMPDIR}//DEBIAN/postinst" 0755

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter"
copyFileToTmpDir "Database.sql" "${NBTMPDIR}/${PACKAGE_TOP_DIR}Database.sql" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//lib/systemd/system"
copyFileToTmpDir "openZmeter.service" "${NBTMPDIR}//lib/systemd/system/openZmeter.service" 0644

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/bin"
copyFileToTmpDir "${OUTPUT_PATH}" "${NBTMPDIR}/${PACKAGE_TOP_DIR}bin/${OUTPUT_BASENAME}" 0755

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/bin"
copyFileToTmpDir "${OUTPUT_PATH}" "${NBTMPDIR}/${PACKAGE_TOP_DIR}bin/${OUTPUT_BASENAME}" 0755

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/bin"
copyFileToTmpDir "${OUTPUT_PATH}" "${NBTMPDIR}/${PACKAGE_TOP_DIR}bin/${OUTPUT_BASENAME}" 0755

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/bin"
copyFileToTmpDir "${OUTPUT_PATH}" "${NBTMPDIR}/${PACKAGE_TOP_DIR}bin/${OUTPUT_BASENAME}" 0755

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/bin"
copyFileToTmpDir "${OUTPUT_PATH}" "${NBTMPDIR}/${PACKAGE_TOP_DIR}bin/${OUTPUT_BASENAME}" 0755

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/bin"
copyFileToTmpDir "${OUTPUT_PATH}" "${NBTMPDIR}/${PACKAGE_TOP_DIR}bin/${OUTPUT_BASENAME}" 0755

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/bin"
copyFileToTmpDir "${OUTPUT_PATH}" "${NBTMPDIR}/${PACKAGE_TOP_DIR}bin/${OUTPUT_BASENAME}" 0755

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/bin"
copyFileToTmpDir "${OUTPUT_PATH}" "${NBTMPDIR}/${PACKAGE_TOP_DIR}bin/${OUTPUT_BASENAME}" 0755

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/bin"
copyFileToTmpDir "${OUTPUT_PATH}" "${NBTMPDIR}/${PACKAGE_TOP_DIR}bin/${OUTPUT_BASENAME}" 0755

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/bin"
copyFileToTmpDir "${OUTPUT_PATH}" "${NBTMPDIR}/${PACKAGE_TOP_DIR}bin/${OUTPUT_BASENAME}" 0755

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/bin"
copyFileToTmpDir "${OUTPUT_PATH}" "${NBTMPDIR}/${PACKAGE_TOP_DIR}bin/${OUTPUT_BASENAME}" 0755

cd "${TOP}"
makeDirectory "${NBTMPDIR}//opt/openZmeter/bin"
copyFileToTmpDir "${OUTPUT_PATH}" "${NBTMPDIR}/${PACKAGE_TOP_DIR}bin/${OUTPUT_BASENAME}" 0755


# Create control file
cd "${TOP}"
CONTROL_FILE=${NBTMPDIR}/DEBIAN/control
rm -f ${CONTROL_FILE}
mkdir -p ${NBTMPDIR}/DEBIAN

cd "${TOP}"
echo 'Package: openZmeter' >> ${CONTROL_FILE}
echo 'Version: 2.0' >> ${CONTROL_FILE}
echo 'Architecture: armhf' >> ${CONTROL_FILE}
echo 'Maintainer: Edu (zRed)' >> ${CONTROL_FILE}
echo 'Description: Power usage and electrical monitoring tool with WEB frontend' >> ${CONTROL_FILE}
echo 'Depends: libpq5 (>= 9.5.10), libjson-c3 (>= 0.12.1), libusb-1.0-0' >> ${CONTROL_FILE}
echo 'Homepage: http://www.openzmeter.com/' >> ${CONTROL_FILE}
echo 'Recommends: chrony' >> ${CONTROL_FILE}

# Create Debian Package
cd "${TOP}"
cd "${NBTMPDIR}/.."
dpkg-deb  --build ${TMPDIRNAME}
checkReturnCode
cd "${TOP}"
mkdir -p  ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/package
mv ${NBTMPDIR}.deb ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/package/openZmeter.deb
checkReturnCode
echo Debian: ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/package/openZmeter.deb

# Cleanup
cd "${TOP}"
rm -rf ${NBTMPDIR}
