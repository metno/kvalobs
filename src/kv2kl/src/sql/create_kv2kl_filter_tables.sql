-- Creates the tables T_KV2KLIMA_FILTER, T_KV2KLIMA_PARAM_FILTER and
-- T_KV2KLIMA_TYPEID_PARAM_FILTER. This tables is used to filter the
-- data form kvalobs to klima. 
--
-- The filter function is implemented
-- in metno.kvalobs.kl.Kv2KlimaFilter, metno.kvalobs.kl.ParamFilter and
-- metno.kvalobs.kl.Filter.


CREATE TABLE T_KV2KLIMA_FILTER (
	stnr   numeric(10,0) NOT NULL,
	status character(1) NOT NULL,
	fdato  timestamp(0),  
	tdato  timestamp(0),
	typeid numeric(4,0),
	nytt_stnr numeric(5,0) 
 );


CREATE TABLE T_KV2KLIMA_PARAM_FILTER (
   	stnr    numeric(10) NOT NULL,
	typeid  numeric(4) NOT NULL,
	paramid numeric(5) NOT NULL,
	sensor  numeric(1) NOT NULL,
	xlevel  integer    NOT NULL,
   	fdato   timestamp(0) DEFAULT NULL,
   	tdato   timestamp(0) DEFAULT NULL
 );

CREATE INDEX T_KV2KLIMA_PARAM_FILTER_TYPEID_INDEX ON T_KV2KLIMA_PARAM_FILTER (stnr,typeid);

CREATE TABLE T_KV2KLIMA_TYPEID_PARAM_FILTER (
	typeid  numeric(4,0)  NOT NULL,
	paramid numeric(5,0)  NOT NULL,
	sensor  numeric(1)    NOT NULL,  
	xlevel  integer       NOT NULL,
	fdato   timestamp(0) DEFAULT NULL,
	tdato   timestamp(0) DEFAULT NULL
 );

CREATE INDEX T_KV2KLIMA_TYPEID_PARAM_FILTER_TYPEID_INDEX ON T_KV2KLIMA_TYPEID_PARAM_FILTER (typeid);
