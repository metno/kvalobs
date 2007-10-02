# Before `make install' is performed this script should be runnable with
# `make test'. After `make install' it should work as `perl test.pl'

#########################

# change 'tests => 1' to 'tests => last_test_to_print';

use ExtUtils::testlib;
use Test;
BEGIN { plan tests => 1 };
use KvService;
ok(1); # If we made it this far, we're ok.

#########################

# Insert your test code below, the Test module is use()ed here so read
# its man page ( perldoc Test ) for help writing this test script.

$result=KvService::getParams();

printf("#Result: %d\n",  $#{$result} );

for $i (@{$result}){
    printf("name: %3s %-3s (%d) [%d]\n", 
	   $i->{'name'},
	   $i->{'unit'},
	   $i->{'paramID'},
	   $i->{'level_scale'});
} 

sub callback{
}

$cb=10;

$id=KvService::subscribeDataNotify($cb);

if($id){
    printf("ID: $id\n");
}else{
    printf("ID: $id  (undef)\n");
}
