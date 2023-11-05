#include <Utils/stdInclude.h>

#pragma warning(push)
#pragma warning(disable: 4805)
#pragma warning(disable: 4005)

#ifdef SR_ICU
    #define U_DISABLE_RENAMING 1
    #define U_COMMON_IMPLEMENTATION 0
    #define U_STATIC_IMPLEMENTATION 1

    #include "../../Utils/libs/icu/unicode/urename.h"
    #include "../../Utils/libs/icu/unicode/uversion.h"

    #include "../../Utils/libs/icu/appendable.cpp"
    #include "../../Utils/libs/icu/bmpset.cpp"
    #include "../../Utils/libs/icu/brkeng.cpp"
    #include "../../Utils/libs/icu/brkiter.cpp"
    #include "../../Utils/libs/icu/bytesinkutil.cpp"
    #include "../../Utils/libs/icu/bytestream.cpp"
    #include "../../Utils/libs/icu/bytestrie.cpp"
    #include "../../Utils/libs/icu/bytestriebuilder.cpp"
    #include "../../Utils/libs/icu/bytestrieiterator.cpp"
    #include "../../Utils/libs/icu/caniter.cpp"
    #include "../../Utils/libs/icu/characterproperties.cpp"
    #include "../../Utils/libs/icu/chariter.cpp"
    #include "../../Utils/libs/icu/charstr.cpp"
    #include "../../Utils/libs/icu/cmemory.cpp"
    #include "../../Utils/libs/icu/cstr.cpp"
    #include "../../Utils/libs/icu/cstring.cpp"
    #include "../../Utils/libs/icu/cwchar.cpp"
    #include "../../Utils/libs/icu/dictbe.cpp"
    #include "../../Utils/libs/icu/dictionarydata.cpp"
    #include "../../Utils/libs/icu/dtintrv.cpp"
    #include "../../Utils/libs/icu/edits.cpp"
    #include "../../Utils/libs/icu/emojiprops.cpp"
    #include "../../Utils/libs/icu/errorcode.cpp"
    #include "../../Utils/libs/icu/filteredbrk.cpp"
    #include "../../Utils/libs/icu/filterednormalizer2.cpp"
    #include "../../Utils/libs/icu/icudataver.cpp"
    #include "../../Utils/libs/icu/icuplug.cpp"
    #include "../../Utils/libs/icu/loadednormalizer2impl.cpp"
    #include "../../Utils/libs/icu/localebuilder.cpp"
    #include "../../Utils/libs/icu/localematcher.cpp"
    #include "../../Utils/libs/icu/localeprioritylist.cpp"
    #include "../../Utils/libs/icu/locavailable.cpp"
    #include "../../Utils/libs/icu/locbased.cpp"
    #include "../../Utils/libs/icu/locdispnames.cpp"
    #include "../../Utils/libs/icu/locdistance.cpp"
    #include "../../Utils/libs/icu/locdspnm.cpp"
    #include "../../Utils/libs/icu/locid.cpp"
    #include "../../Utils/libs/icu/loclikely.cpp"
    #include "../../Utils/libs/icu/loclikelysubtags.cpp"
    #include "../../Utils/libs/icu/locmap.cpp"
    #include "../../Utils/libs/icu/locresdata.cpp"
    #include "../../Utils/libs/icu/locutil.cpp"
    #include "../../Utils/libs/icu/lsr.cpp"
    #include "../../Utils/libs/icu/lstmbe.cpp"
    #include "../../Utils/libs/icu/messagepattern.cpp"
    #include "../../Utils/libs/icu/normalizer2.cpp"
    #include "../../Utils/libs/icu/normalizer2impl.cpp"
    #include "../../Utils/libs/icu/normlzr.cpp"
    #include "../../Utils/libs/icu/parsepos.cpp"
    #include "../../Utils/libs/icu/patternprops.cpp"
    #include "../../Utils/libs/icu/pluralmap.cpp"
    #include "../../Utils/libs/icu/propname.cpp"
    #include "../../Utils/libs/icu/propsvec.cpp"
    #include "../../Utils/libs/icu/punycode.cpp"
    #include "../../Utils/libs/icu/putil.cpp"
    #include "../../Utils/libs/icu/rbbi.cpp"
    #include "../../Utils/libs/icu/rbbidata.cpp"
    #include "../../Utils/libs/icu/rbbinode.cpp"
    #include "../../Utils/libs/icu/rbbirb.cpp"
    #include "../../Utils/libs/icu/rbbiscan.cpp"
    #include "../../Utils/libs/icu/rbbisetb.cpp"
    #include "../../Utils/libs/icu/rbbistbl.cpp"
    #include "../../Utils/libs/icu/rbbitblb.cpp"
    #include "../../Utils/libs/icu/rbbi_cache.cpp"
    #include "../../Utils/libs/icu/resbund.cpp"
    #include "../../Utils/libs/icu/resbund_cnv.cpp"
    #include "../../Utils/libs/icu/resource.cpp"
    #include "../../Utils/libs/icu/restrace.cpp"
    #include "../../Utils/libs/icu/ruleiter.cpp"
    #include "../../Utils/libs/icu/schriter.cpp"
    #include "../../Utils/libs/icu/serv.cpp"
    #include "../../Utils/libs/icu/servlk.cpp"
    #include "../../Utils/libs/icu/servlkf.cpp"
    #include "../../Utils/libs/icu/servls.cpp"
    #include "../../Utils/libs/icu/servnotf.cpp"
    #include "../../Utils/libs/icu/servrbf.cpp"
    #include "../../Utils/libs/icu/servslkf.cpp"
    #include "../../Utils/libs/icu/sharedobject.cpp"
    #include "../../Utils/libs/icu/simpleformatter.cpp"
    #include "../../Utils/libs/icu/static_unicode_sets.cpp"
    #include "../../Utils/libs/icu/stringpiece.cpp"
    #include "../../Utils/libs/icu/stringtriebuilder.cpp"
    #include "../../Utils/libs/icu/uarrsort.cpp"
    #include "../../Utils/libs/icu/ubidi.cpp"
    #include "../../Utils/libs/icu/ubidiln.cpp"
    #include "../../Utils/libs/icu/ubiditransform.cpp"
    #include "../../Utils/libs/icu/ubidiwrt.cpp"
    #include "../../Utils/libs/icu/ubidi_props.cpp"
    #include "../../Utils/libs/icu/ubrk.cpp"
    #include "../../Utils/libs/icu/ucase.cpp"
    #include "../../Utils/libs/icu/ucasemap.cpp"
    #include "../../Utils/libs/icu/ucasemap_titlecase_brkiter.cpp"
    #include "../../Utils/libs/icu/ucat.cpp"
    #include "../../Utils/libs/icu/uchar.cpp"
    #include "../../Utils/libs/icu/ucharstrie.cpp"
    #include "../../Utils/libs/icu/ucharstriebuilder.cpp"
    #include "../../Utils/libs/icu/ucharstrieiterator.cpp"
    #include "../../Utils/libs/icu/uchriter.cpp"
    #include "../../Utils/libs/icu/ucln_cmn.cpp"
    #include "../../Utils/libs/icu/ucmndata.cpp"
    #include "../../Utils/libs/icu/ucnv.cpp"
    #include "../../Utils/libs/icu/ucnv2022.cpp"
    #include "../../Utils/libs/icu/ucnvbocu.cpp"
    #include "../../Utils/libs/icu/ucnvdisp.cpp"
    #include "../../Utils/libs/icu/ucnvhz.cpp"
    #include "../../Utils/libs/icu/ucnvisci.cpp"
    #include "../../Utils/libs/icu/ucnvlat1.cpp"
    #include "../../Utils/libs/icu/ucnvmbcs.cpp"
    #include "../../Utils/libs/icu/ucnvscsu.cpp"
    #include "../../Utils/libs/icu/ucnvsel.cpp"
    #include "../../Utils/libs/icu/ucnv_bld.cpp"
    #include "../../Utils/libs/icu/ucnv_cb.cpp"
    #include "../../Utils/libs/icu/ucnv_cnv.cpp"
    #include "../../Utils/libs/icu/ucnv_ct.cpp"
    #include "../../Utils/libs/icu/ucnv_err.cpp"
    #include "../../Utils/libs/icu/ucnv_ext.cpp"
    #include "../../Utils/libs/icu/ucnv_io.cpp"
    #include "../../Utils/libs/icu/ucnv_lmb.cpp"
    #include "../../Utils/libs/icu/ucnv_set.cpp"
    #include "../../Utils/libs/icu/ucnv_u16.cpp"
    #include "../../Utils/libs/icu/ucnv_u32.cpp"
    #include "../../Utils/libs/icu/ucnv_u7.cpp"
    #include "../../Utils/libs/icu/ucnv_u8.cpp"
    #include "../../Utils/libs/icu/ucol_swp.cpp"
    #include "../../Utils/libs/icu/ucptrie.cpp"
    #include "../../Utils/libs/icu/ucurr.cpp"
    #include "../../Utils/libs/icu/udata.cpp"
    #include "../../Utils/libs/icu/udatamem.cpp"
    #include "../../Utils/libs/icu/udataswp.cpp"
    #include "../../Utils/libs/icu/uenum.cpp"
    #include "../../Utils/libs/icu/uhash.cpp"
    #include "../../Utils/libs/icu/uhash_us.cpp"
    #include "../../Utils/libs/icu/uidna.cpp"
    #include "../../Utils/libs/icu/uinit.cpp"
    #include "../../Utils/libs/icu/uinvchar.cpp"
    #include "../../Utils/libs/icu/uiter.cpp"
    #include "../../Utils/libs/icu/ulist.cpp"
    #include "../../Utils/libs/icu/uloc.cpp"
    #include "../../Utils/libs/icu/uloc_keytype.cpp"
    #include "../../Utils/libs/icu/uloc_tag.cpp"
    #include "../../Utils/libs/icu/umapfile.cpp"
    #include "../../Utils/libs/icu/umath.cpp"
    #include "../../Utils/libs/icu/umutablecptrie.cpp"
    #include "../../Utils/libs/icu/umutex.cpp"
    #include "../../Utils/libs/icu/unames.cpp"
    #include "../../Utils/libs/icu/unifiedcache.cpp"
    #include "../../Utils/libs/icu/unifilt.cpp"
    #include "../../Utils/libs/icu/unifunct.cpp"
    #include "../../Utils/libs/icu/uniset.cpp"
    #include "../../Utils/libs/icu/unisetspan.cpp"
    #include "../../Utils/libs/icu/uniset_closure.cpp"
    #include "../../Utils/libs/icu/uniset_props.cpp"
    #include "../../Utils/libs/icu/unistr.cpp"
    #include "../../Utils/libs/icu/unistr_case.cpp"
    #include "../../Utils/libs/icu/unistr_case_locale.cpp"
    #include "../../Utils/libs/icu/unistr_cnv.cpp"
    #include "../../Utils/libs/icu/unistr_props.cpp"
    #include "../../Utils/libs/icu/unistr_titlecase_brkiter.cpp"
    #include "../../Utils/libs/icu/unorm.cpp"
    #include "../../Utils/libs/icu/unormcmp.cpp"
    #include "../../Utils/libs/icu/uobject.cpp"
    #include "../../Utils/libs/icu/uprops.cpp"
    #include "../../Utils/libs/icu/uresbund.cpp"
    #include "../../Utils/libs/icu/uresdata.cpp"
    #include "../../Utils/libs/icu/ures_cnv.cpp"
    #include "../../Utils/libs/icu/uscript.cpp"
    #include "../../Utils/libs/icu/uscript_props.cpp"
    #include "../../Utils/libs/icu/usc_impl.cpp"
    #include "../../Utils/libs/icu/uset.cpp"
    #include "../../Utils/libs/icu/usetiter.cpp"
    #include "../../Utils/libs/icu/uset_props.cpp"
    #include "../../Utils/libs/icu/ushape.cpp"
    #include "../../Utils/libs/icu/usprep.cpp"
    #include "../../Utils/libs/icu/ustack.cpp"
    #include "../../Utils/libs/icu/ustrcase.cpp"
    #include "../../Utils/libs/icu/ustrcase_locale.cpp"
    #include "../../Utils/libs/icu/ustrenum.cpp"
    #include "../../Utils/libs/icu/ustrfmt.cpp"
    #include "../../Utils/libs/icu/ustring.cpp"
    #include "../../Utils/libs/icu/ustrtrns.cpp"
    #include "../../Utils/libs/icu/ustr_cnv.cpp"
    #include "../../Utils/libs/icu/ustr_titlecase_brkiter.cpp"
    #include "../../Utils/libs/icu/ustr_wcs.cpp"
    #include "../../Utils/libs/icu/utext.cpp"
    #include "../../Utils/libs/icu/utf_impl.cpp"
    #include "../../Utils/libs/icu/util.cpp"
    #include "../../Utils/libs/icu/util_props.cpp"
    #include "../../Utils/libs/icu/utrace.cpp"
    #include "../../Utils/libs/icu/utrie.cpp"
    #include "../../Utils/libs/icu/utrie2.cpp"
    #include "../../Utils/libs/icu/utrie2_builder.cpp"
    #include "../../Utils/libs/icu/utrie_swap.cpp"
    #include "../../Utils/libs/icu/uts46.cpp"
    #include "../../Utils/libs/icu/utypes.cpp"
    #include "../../Utils/libs/icu/uvector.cpp"
    #include "../../Utils/libs/icu/uvectr32.cpp"
    #include "../../Utils/libs/icu/uvectr64.cpp"

    #ifdef SR_WIN32
        #include "../Utils/libs/icu/wintz.cpp"
    #endif
#endif

#pragma warning(pop)
