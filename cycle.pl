#Arai RyosukeS
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
unlink glob ("*.txt");


for($i=1;$i<101;$i++){

	printf "\n\n\nNo.%d\n", ($i);
	
	$result = system("./Rand");

	$result = system("./EDF");
	
	$result = system("./EDZL");
	
	$result = system("./LLF");
	
	printf "\n\n\nNo.%d\n", ($i);
	
	$result = system("./LMCF");
	
	printf "\n\n\nNo.%d\n", ($i);
	
	$result = system("./LMCLF_no1");
	
	$result = system("./LMCLF_no2");
	
	$result = system("./LMCLF_no3");
	
	printf "\n\n\nNo.%d\n", ($i);
	
	$result = system("./AcII_LMCF");
	
	printf "\n\n\nNo.%d\n", ($i);
	
	$result = system("./AcII_LMCLF_no1");
	
	$result = system("./AcII_LMCLF_no2");
	
	$result = system("./AcII_LMCLF_no3");
	
	printf "\n\n\nNo.%d\n", ($i);
	
	$result = system("./LLF_LMCLF_no1");
	
	$result = system("./LLF_LMCLF_no2");
	
	$result = system("./LLF_LMCLF_no3");
	
	printf "\n\n\nNo.%d\n", ($i);
	
	$result = system("./LLF_AcII_LMCLF_no1");
	
	$result = system("./LLF_AcII_LMCLF_no2");
	
	$result = system("./LLF_AcII_LMCLF_no3");
	
	$result = system("./Result");
	
	#タスクセットの保存
	@files = glob ("*.txt");
	foreach $j (@files){
		mkdir("./Task_Set_$i");
		copy("$j", "./Task_Set_$i");
	}

}

#メールの送信
my $from    = 'arai@sos.info.hiroshima-cu.ac.jp';
my $mailto  = 'arai@sos.info.hiroshima-cu.ac.jp';
my $subject = Encode::encode('MIME-Header-ISO_2022_JP', 'Experiment Finishes');
my $header  = "From: $from\n" .
              "To: $mailto\n" .
              "Subject: $subject\n" .
              "Mime-Version: 1.0\n" .
              "Content-Type: text/plain; charset = ISO-2022-JP\n" .
              "Content-Trensfer-Encoding: 7bit\n";
my $message = Encode::encode('iso-2022-jp', 'This message has no content.');


my $smtp = Net::SMTP->new('mail.sos.info.hiroshima-cu.ac.jp');
if ( !$smtp ) {
     print encode('iso-2022-jp',  "Can't connect mail server\n");
     exit;
}


$smtp->mail($from);
$smtp->to($mailto);
$smtp->data();
$smtp->datasend($header);
$smtp->datasend($message);
$smtp->dataend();
$smtp->quit;
