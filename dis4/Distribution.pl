#Machigasira Yuki
#! C:\Perl\bin\perl.exe
#����perl���s�t�@�C���̐�΃p�X��


use strict; 					#���܂��Ȃ��@�L�q�~�X�Ȃǔ�������
use warnings;					#���܂��Ȃ��@�x���̏o��
use File::Copy;
use Fcntl;
use Encode;
use Net::SMTP;
use utf8;

my $result;
my $i;
my $j;
my @files;

#������
#unlink glob ("*.txt");


for($i=1;$i<50001;$i++){

	printf "\n\n\nNo.%d\n", ($i);

	$result = system("./Distribution");

}

