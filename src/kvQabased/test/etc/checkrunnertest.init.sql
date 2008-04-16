insert into model_data values ( 42, '2006-05-26 06:00:00', 110, 0, 20, 10.3 );
insert into model_data values ( 42, '2006-05-26 06:00:00', 111, 0, 20, 11 );
insert into model_data values (  9, '2006-05-26 06:00:00', 111, 0, 20, 12 );


insert into station_param values (0,33,0,0,1,365,-1,'QC1-1-33','max;highest;high;low;lowest;min
29.0;29.0;29.0;1.0;1.0;1.0','','1500-01-01 00:00:00' );
insert into station_param values (0,110,0,0,1,365,-1,'QC1-4-110','highest;high;low;lowest;dry
13.20;6.82;-4.06;-11.30;1.0', '', '1500-01-01 00:00:00' );
insert into station_param values (0,110,25,0,1,365,-1,'QC1-4-110-2','highest;high;low;lowest;dry
13.20;6.82;-4.06;-11.30;1.0', '', '1500-01-01 00:00:00' );
insert into station_param values (0,110,0,1,1,365,-1,'QC1-4-110-3','highest;high;low;lowest;dry
13.20;6.82;-4.06;-11.30;1.0', '', '1500-01-01 00:00:00' );
insert into station_param values (0,111,0,0,1,365,-1,'QC1-4-111','highest;high;low;lowest;dry
13.20;6.82;-4.06;-11.30;1.0', '', '1500-01-01 00:00:00' );
insert into station_param values (0,112,0,0,1,365,-1,'QC1-1-34','max;highest;high;low;lowest;min
29.0;29.0;29.0;1.0;1.0;1.0','','1500-01-01 00:00:00' );
insert into station_param values (0,42,0,0,1,365,-1,'QC1-1-35','max;highest;high;low;lowest;min
29.0;29.0;29.0;1.0;1.0;1.0','','1500-01-01 00:00:00' );



insert into station values ( 42, 59.9427, 10.7207, 94, 0, 'Min stasjon', 1, 1,'','','',8,'t','1996-12-01 00:00:00' );
insert into station values (  9, 59.9427, 10.7207, 94, 0, 'Min stasjon 2', 1, 1,'','','',8,'t','1996-12-01 00:00:00' );


insert into param values (33, 'V3', 'Været ved observasjonstiden, tredje tegn', 'nasjonal kode to siffer', 0, '' );
insert into param values (110, 'RR_24', 'Nedbør, tilvekst siste 24 timer', 'mm', 0, '' );
insert into param values (111, 'R', 'Nedbør, tilvekst siste 14 timer', 'mm', 0, '' );
insert into param values (112, 'SA', 'Snødybde', 'cm',0, 'nåverdi, totalt fra bakken' );
insert into param values (42, 'W1', 'Været siden forrige hovedobservasjonstid', 'WMO-kode 4561 ett siffer', 0, '' );


insert into obs_pgm values (42, 33, 0, 1, 302, 't', 't', 't', 't', 't', 't', 't', 't', 't', 't', 't', 't', 't', 't', 't', 't', 't', 't', 't', 't', 't', 't', 't', 't', 't', 't', 't', 't', 't', 't',  't', 't', '2006-05-26 06:00:00');
insert into obs_pgm values (42, 110, 0, 1, 302, 't', 't', 't', 't', 't', 't', 't', 't', 't', 't', 't', 't', 't', 't', 't', 't', 't', 't', 't', 't', 't', 't', 't', 't', 't', 't', 't', 't', 't', 't',  't', 't', '2006-05-26 06:00:00');
insert into obs_pgm values (9 , 111, 0, 1, 303, 't', 't', 't', 't', 't', 't', 't', 't', 't', 't', 't', 't', 't', 't', 't', 't', 't', 't', 't', 't', 't', 't', 't', 't', 't', 't', 't', 't', 't', 't',  't', 't', '2006-05-26 06:00:00');
insert into obs_pgm values (42, 112, 0, 1, 302, 't', 't', 't', 't', 't', 't', 't', 't', 't', 't', 't', 't', 't', 't', 't', 't', 't', 't', 't', 't', 't', 't', 't', 't', 't', 't', 't', 't', 't', 't',  't', 't', '2006-05-26 06:00:00');
insert into obs_pgm values (42, 42, 0, 1, 302, 't', 't', 't', 't', 't', 't', 't', 't', 't', 't', 't', 't', 't', 't', 't', 't', 't', 't', 't', 't', 't', 't', 't', 't', 't', 't', 't', 't', 't', 't',  't', 't', '2006-05-26 06:00:00');


insert into checks values (0, 'QC1-1-33', 'QC1-1',1,'RANGE_CHECK','obs;V3;;|meta;V3_max,V3_highest,V3_high,V3_low,V3_lowest,V3_min;;','* * * * *' , '1500-01-01 00:00:00' );
insert into checks values (0, 'QC1-4-110', 'QC1-4', 1, 'PROGNOSTIC SPACE_CHECK_RR' ,'obs;RR_24&0&&;;|model;RR_24&0&&;;|meta;RR_24_highest,RR_24_high,RR_24_low,RR_24_lowest,RR_24_dry;;', '* * * * *', '1500-01-01 00:00:00' );
insert into checks values (0, 'QC1-4-110-2', 'QC1-4', 1, 'PROGNOSTIC SPACE_CHECK_RR' ,'obs;RR_24&25&&;;|model;RR_24&0&&;;|meta;RR_24_highest,RR_24_high,RR_24_low,RR_24_lowest,RR_24_dry;;', '* * * * *', '1500-01-01 00:00:00' );
insert into checks values (0, 'QC1-4-110-3', 'QC1-4', 1, 'PROGNOSTIC SPACE_CHECK_RR' ,'obs;RR_24&&1&;;|model;RR_24&0&&;;|meta;RR_24_highest,RR_24_high,RR_24_low,RR_24_lowest,RR_24_dry;;', '* * * * *', '1500-01-01 00:00:00' );
insert into checks values (0, 'QC1-4-111', 'QC1-4', 1, 'PROGNOSTIC SPACE_CHECK_RR' ,'obs;R&0&&303;;|model;R&0&&;;|meta;R_highest,R_high,R_low,R_lowest,R_dry;;', '* * * * *', '1500-01-01 00:00:00' );
insert into checks values (0, 'QC1-1-34', 'QC1-1',1,'RANGE_CHECK','obs;SA;;|meta;SA_max,SA_highest,SA_high,SA_low,SA_lowest,SA_min;;','* * * * *' , '1500-01-01 00:00:00' );
insert into checks values (0, 'QC1-1-35', 'QC1-1',1,'RANGE_CHECK','obs;W1;;|meta;W1_max,W1_highest,W1_high,W1_low,W1_lowest,W1_min;;','* * * * *' , '1500-01-01 00:00:00' );
insert into checks values (0, 'QC1-2-112', 'QC1-2', 1, 'EQUAL_CHECK', 'obs;SA,W1;;', '* * * * *' , '1500-01-01 00:00:00' );



insert into qcx_info values('QC1-9', 'QC1', 14, 'Kombinert vurdering');
insert into qcx_info values('QC1-0', 'QC1', 13, 'Forhandskvalifisering');
insert into qcx_info values('QC1-7', 'QC1', 12, 'Identifisering av oppsamlet verdi');
insert into qcx_info values('QC2m-2', 'QC2m', 12, 'Fordeling av oppsamlet verdi');
insert into qcx_info values('QC1-1', 'QC1', 1, 'Grenseverdikontroll');
insert into qcx_info values('QC1-2', 'QC1', 2, 'Formell konsistenskontroll');
insert into qcx_info values('QC1-3a', 'QC1', 3, 'Sprangkontroll, step');
insert into qcx_info values('QC1-3b', 'QC1', 3, 'Sprangkontroll, freeze');
insert into qcx_info values('QC1-4', 'QC1', 4, 'Prognostisk romkontroll');
insert into qcx_info values('QC1-5', 'QC1', 5, 'Meldingskontroll');
insert into qcx_info values('QC1-6', 'QC1', 10, 'Klimatologisk konsistenskontroll');
insert into qcx_info values('QC2d-1', 'QC2d', 3, 'Sprangkontroll, dip');
insert into qcx_info values('QC2d-2', 'QC2d', 7, 'Tidsserietilpasning');
insert into qcx_info values('QC2d-3', 'QC2d', 8, 'Vranalyse');
insert into qcx_info values('QC2d-4', 'QC2d', 9, 'Statistikkontroll');
insert into qcx_info values('QC2m-1', 'QC2m', 11, 'Klimatologikontroll');
insert into qcx_info values('HQC', 'HQC', 15, 'HQC');


insert into algorithms values (1,'RANGE_CHECK', 'obs;X;;|meta;X_1,X_2,X_3,X_4,X_5,X_6;;','
# checkname:          RANGE_CHECK
# signature:             obs;X;;|meta;X_1,X_2,X_3,X_4,X_5,X_6;;
# Grenseverdikontroll
# gjør noen tester og returner en ny verdi for flag

sub check {
#tolererer ingen manglende observasjoner:
   #if ( $X_missing[0] > 0 ) {
   #   return 0;
   # }

    my $flag = 1;

    if ( $X[0] < $X_6[0] || $X[0] > $X_1[0] ) {
        $flag = 6;
        $X_missing[0] = 2;
    }
    elsif ( $X[0] <= $X_1[0] && $X[0] > $X_2[0] ) {
        $flag = 4;
    }
    elsif ( $X[0] <= $X_2[0] && $X[0] > $X_3[0] ) {
        $flag = 2;
    }
    elsif ( $X[0] < $X_4[0] && $X[0] >= $X_5[0] ) {
        $flag = 3;
    }
    elsif ( $X[0] < $X_5[0] && $X[0] >= $X_6[0] ) {
        $flag = 5;
    }

    my @retvector;
    push(@retvector,"X_0_0_flag");
    push(@retvector,$flag);

    if ( $X_missing[0] > 0 ) {
       push(@retvector,"X_0_0_missing");
       push(@retvector,$X_missing[0]);
   }

    my $numout = @retvector;

    return (@retvector, $numout);
}
');
insert into algorithms values (1, 'PROGNOSTIC SPACE_CHECK_RR', 'obs;X;;|model;mX;;|meta;X_1,X_2,X_3,X_4,X_dry;;','
#checkname:  PROGNOSTIC SPACE_CHECK_RR
#signature:  obs;X;;|model;mX;;|meta;X_1,X_2,X_3,X_4,X_dry;;

# PROGNOSTIC SPACE_CHECK OF PRECIPITATION
# Dry conditions added 27 May 2005 by Gabriel Kielland


sub check {

    my $flag = 1;

    if ( $X_missing[0] == 2 || $X_missing[0] == 3 ) {
       if ( $mX[0] < $X_dry[0] ) {
            $X[0] = -1;
       }
       else {
            $X[0] = $mX[0];
       }
       $flag = 6;
       $X_missing[0]-=2;
    }

    elsif ( $X_missing[0] < 2 ) {

        if ( ($X[0] - $mX[0]) >= $X_1[0] ) {
            $flag = 4;
        }
        elsif ( ($X[0] - $mX[0]) < $X_1[0] && ($X[0] - $mX[0]) >= $X_2[0] ) {
            $flag = 2;
        }
        elsif ( ($X[0] - $mX[0]) < $X_3[0] && ($X[0] - $mX[0]) >= $X_4[0] ) {
            $flag = 3;
        }
        elsif ( ($X[0] - $mX[0]) < $X_4[0] ) {
            $flag = 5;
        }
    }

    my @retvector = ("X_0_0_flag", $flag);
    push(@retvector, "X_0_0_corrected");
    push(@retvector, $X[0]);
    push(@retvector, "X_0_0_missing");
    push(@retvector, $X_missing[0]);
    my $numout = @retvector;

    return (@retvector, $numout);
}
'
);

insert into algorithms values (1, 'EQUAL_CHECK', 'obs;X,Y;;','
#checkname:  EQUAL_CHECK
#signature: obs;X,Y;;

# check for unit tests


sub check {

    my $flag = 1;

	if ( $Y_missing[0] == 3 ) {
		$flag = 8;
	}
    elsif ( $X[0] != $Y[0] ) {
		$flag = 9;
    }
	else {
		$flag = 2;
	}

    my @retvector = ("X_0_0_flag", $flag);
    my $numout = @retvector;

    return (@retvector, $numout);
}
'
);