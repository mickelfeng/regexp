<?php
/* ==================== INI ==================== */

ini_set('intl.error_level', E_WARNING);

/* ==================== VAR ==================== */

$A = "\xF0\x9D\x98\xBC"; # 1D63C, Lu
$B = "\xF0\x9D\x98\xBD"; # 1D63D
$C = "\xF0\x9D\x98\xBE"; # 1D63E
$D = "\xF0\x9D\x98\xBF"; # 1D63F
$E = "\xF0\x9D\x99\x80"; # 1D640
$F = "\xF0\x9D\x99\x81"; # 1D641
$G = "\xF0\x9D\x99\x82"; # 1D642
$H = "\xF0\x9D\x99\x83"; # 1D643

$a = "\xF0\x9D\x98\xA2"; # 1D622
$d = "\xF0\x9D\x98\xA5"; # 1D625
$e = "\xF0\x9D\x98\xA6"; # 1D626
$g = "\xF0\x9D\x98\xA8"; # 1D628
$h = "\xF0\x9D\x98\xA9"; # 1D629
$i = "\xF0\x9D\x98\xAA"; # 1D62A
$l = "\xF0\x9D\x98\xAD"; # 1D62D
$n = "\xF0\x9D\x98\xAF"; # 1D62F
$o = "\xF0\x9D\x98\xB0"; # 1D630
$r = "\xF0\x9D\x98\xB3"; # 1D633
$s = "\xF0\x9D\x98\xB4"; # 1D634
$t = "\xF0\x9D\x98\xB5"; # 1D635
$w = "\xF0\x9D\x98\xB8"; # 1D638

$N0="\xF0\x9D\x9F\x8E"; # 1D7CE, Nd
$N1="\xF0\x9D\x9F\x8F";
$N2="\xF0\x9D\x9F\x90";
$N3="\xF0\x9D\x9F\x91";
$N4="\xF0\x9D\x9F\x92";
$N5="\xF0\x9D\x9F\x93";
$N6="\xF0\x9D\x9F\x94";
$N7="\xF0\x9D\x9F\x95";
$N8="\xF0\x9D\x9F\x96";
$N9="\xF0\x9D\x9F\x97";

$ff = "\xEF\xAC\x80";  # FB00
$fi = "\xEF\xAC\x81";  # FB01
$ffi = "\xEF\xAC\x83"; # FB03

// Prefixes: N for not case folded ; C for case folded equivalent

# 03B0 (2) <=> 03C5 0308 0301 (6)
$N03B0 = "\xCE\xB0";
$C03B0 = "\xCF\x85\xCC\x88\xCC\x81";
# 0587 (2) <=> 0565 0582 (4)
$N0587 = "\xD6\x87";
$C0587 = "\xD5\xA5\xD6\x82";
# 1FB7 (3) <=> 03B1 0342 03B9 (6)
$N1FB7 = "\xE1\xBE\xB7";
$C1FB7 = "\xCE\xB1\xCD\x82\xCE\xB9";

/* ==================== FUNC ==================== */

function ut_regexp_create($oo)
{
    $args = func_get_args();
    array_splice($args, 0, 1);
    if ($oo) {
        $r = new ReflectionClass('Regexp');
        return $r->newInstanceArgs($args);
    } else {
        return call_user_func_array('regexp_create', $args);
    }
}

function ut_regexp_split($oo, $ro)
{
    $args = func_get_args();
    array_splice($args, 0, $oo ? 2 : 1);
    return call_user_func_array($oo ? array($ro, 'split') : 'regexp_split', $args);
}

