--TEST--
Test utf8_slice_replace function
--SKIPIF--
<?php if (!extension_loaded('intl')) echo 'skip'; ?>
--FILE--
<?php
ini_set('intl.error_level', E_WARNING);

$A="\xF0\x9D\x98\xBC"; # 1D63C, Lu
$B="\xF0\x9D\x98\xBD"; # 1D63D
$C="\xF0\x9D\x98\xBE"; # 1D63E
$D="\xF0\x9D\x98\xBF"; # 1D63F
$E="\xF0\x9D\x99\x80"; # 1D640
$F="\xF0\x9D\x99\x81"; # 1D641
$G="\xF0\x9D\x99\x82"; # 1D642
$H="\xF0\x9D\x99\x83"; # 1D643
$I="\xF0\x9D\x99\x84"; # 1D644
$J="\xF0\x9D\x99\x85"; # 1D645
$K="\xF0\x9D\x99\x86"; # 1D646
$L="\xF0\x9D\x99\x87"; # 1D647
$M="\xF0\x9D\x99\x88"; # 1D648
$N="\xF0\x9D\x99\x89"; # 1D649
$O="\xF0\x9D\x99\x8A"; # 1D64A
$P="\xF0\x9D\x99\x8B"; # 1D64B
$Q="\xF0\x9D\x99\x8C"; # 1D64C
$R="\xF0\x9D\x99\x8D"; # 1D64D
$S="\xF0\x9D\x99\x8E"; # 1D64E
$T="\xF0\x9D\x99\x8F"; # 1D64F
$U="\xF0\x9D\x99\x90"; # 1D650
$V="\xF0\x9D\x99\x91"; # 1D651
$W="\xF0\x9D\x99\x92"; # 1D652
$X="\xF0\x9D\x99\x93"; # 1D653
$Y="\xF0\x9D\x99\x94"; # 1D654
$Z="\xF0\x9D\x99\x95"; # 1D655

var_dump(utf8_slice_replace('foo', 'bar'));
var_dump(utf8_slice_replace('foo', 'bar', 0, 0, TRUE));

$string = "$A$B$C$D$E$F$G$H$M$N$R$P$Q$R";
$substring = "$X$Y$Z";

echo 'Replace whole string by substring', "\n";
var_dump(utf8_slice_replace($string, $substring, 0) === $substring);
var_dump(utf8_slice_replace($string, $substring, 0, 14/*utf8_len($string)*/) === $substring);

echo 'Prepend substring to string', "\n";
var_dump(utf8_slice_replace($string, $substring, 0, 0) === $substring . $string);

echo 'Replace DEFGHMNR by IJKL (all possible variations)', "\n";
var_dump(utf8_slice_replace($string, "$I$J$K$L", 3, 8) === "$A$B$C$I$J$K$L$P$Q$R");
var_dump(utf8_slice_replace($string, "$I$J$K$L", 3, -3) === "$A$B$C$I$J$K$L$P$Q$R");
var_dump(utf8_slice_replace($string, "$I$J$K$L", -11, 8) === "$A$B$C$I$J$K$L$P$Q$R");
var_dump(utf8_slice_replace($string, "$I$J$K$L", -11, -3) === "$A$B$C$I$J$K$L$P$Q$R");

echo 'Replace DEF by ZYXWVUTSRQPOLKJIHGFEDCBA (all possible variations)', "\n";
$substring = "$Z$Y$X$W$V$U$T$S$R$Q$P$O$L$K$J$I$H$G$F$E$D$C$B$A";
var_dump(utf8_slice_replace($string, $substring, 3, 3) === "$A$B$C$substring$G$H$M$N$R$P$Q$R");
var_dump(utf8_slice_replace($string, $substring, 3, -8) === "$A$B$C$substring$G$H$M$N$R$P$Q$R");
var_dump(utf8_slice_replace($string, $substring, -11, 3) === "$A$B$C$substring$G$H$M$N$R$P$Q$R");
var_dump(utf8_slice_replace($string, $substring, -11, -8) === "$A$B$C$substring$G$H$M$N$R$P$Q$R");

echo 'Delete CDEFGH (all possible variations)', "\n";
var_dump(utf8_slice_replace($string, '', 2, 6) === "$A$B$M$N$R$P$Q$R");
var_dump(utf8_slice_replace($string, '', 2, -6) === "$A$B$M$N$R$P$Q$R");
var_dump(utf8_slice_replace($string, '', -12, 6) === "$A$B$M$N$R$P$Q$R");
var_dump(utf8_slice_replace($string, '', -12, -6) === "$A$B$M$N$R$P$Q$R");
?>
--EXPECTF--

Warning: utf8_slice_replace() expects at least 3 parameters, 2 given in %s on line %d
NULL

Warning: utf8_slice_replace() expects at most 4 parameters, 5 given in %s on line %d
NULL
Replace whole string by substring
bool(true)
bool(true)
Prepend substring to string
bool(true)
Replace DEFGHMNR by IJKL (all possible variations)
bool(true)
bool(true)
bool(true)
bool(true)
Replace DEF by ZYXWVUTSRQPOLKJIHGFEDCBA (all possible variations)
bool(true)
bool(true)
bool(true)
bool(true)
Delete CDEFGH (all possible variations)
bool(true)
bool(true)
bool(true)
bool(true)
