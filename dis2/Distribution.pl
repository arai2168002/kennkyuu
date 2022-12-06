#Machigasira Yuki
#! C:\Perl\bin\perl.exe
#↑はperl実行ファイルの絶対パス名


use strict; 					#おまじない　記述ミスなど発見する
use warnings;					#おまじない　警告の出力
use File::Copy;
use Fcntl;
use Encode;
use Net::SMTP;
use utf8;

my $result;
my $i;
my $j;
my @files;

#初期化
#unlink glob ("*.txt");


for($i=1;$i<50001;$i++){

	printf "\n\n\nNo.%d\n", ($i);

	$result = system("./Distribution");

}

