begin;

alter table rejectdecode add fixed boolean not null default false;

commit;