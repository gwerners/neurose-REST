#include "hiredis.h"

#define ID           1
#define NUMBER       2
#define REFERENCE    3
#define STATUS       4
#define CREATED_AT   5
#define UPDATED_AT   6
#define ITEMS        7
#define NOTES        8
#define TRANSACTIONS 9
#define PRICE        10

#define SKU        11
#define UNIT_PRICE 12
#define QUANTITY   13

#define TID                14
#define EXTERNAL_ID        15
#define AMOUNT             16
#define TYPE               17
#define AUTHORIZATION_CODE 18
#define CARD_BRAND         19
#define CARD_BIN           20
#define CARD_LAST          21


//Status
#define STATUS_DRAFT      1
#define STATUS_ENTERED    1<<2
#define STATUS_CANCELED   1<<3
#define STATUS_PAID       1<<4
#define STATUS_APPROVED   1<<5
#define STATUS_REJECTED   1<<6
#define STATUS_RE_ENTERED 1<<7
#define STATUS_CLOSED     1<<8

//Type
#define TYPE_PAYMENT 1
#define TYPE_CANCEL  1<<2


#define MAX_TOKENS 1024
#define debug 0
#define IFDBG if(debug)

#define GET_METHOD    14
#define POST_METHOD   15
#define PUT_METHOD    16
#define DELETE_METHOD 17

#define GET_ALL      1
#define GET_BY_VALUE 2
#define GET_BY_ID    3
#define GET_ITEMS    4
#define GET_TRANSACTIONS 5

#define PUT_ALL      1
#define PUT_BY_ID    2
#define PUT_TRANSACTIONS 3

typedef struct {
    int start;
    int end;
}entry;

typedef struct {
	entry id;
	entry tid;
}request;

typedef struct {
    entry sku;
    entry unit_price;
    entry quantity;
}item;

typedef struct t_item_list{
    item i;
    struct t_item_list *next;
}item_list;

typedef struct {
    entry tid;
    entry external_id;
    entry amount;
    entry type;
    entry authorization_code;
    entry card_brand;
    entry card_bin;
    entry card_last;
}transaction;

typedef struct t_transaction_list{
    transaction t;
    struct t_transaction_list *next;
}transaction_list;

typedef struct
{
   entry id;
   entry number;
   entry reference;
   entry status;
   entry created_at;
   entry updated_at;
   item_list *items;
   entry notes;
   transaction_list *transactions;
   entry price;
} jsonEntry;

//validate http request
int parse_method(const char * cursor, const char * const limit);

//validate json
int parse_items(const char * cursor, const char * const limit);
int parse_transactions(const char * cursor, const char * const limit);
int parse_id(const char * cursor, const char * const limit);
int check_uuid(const char * cursor, const char * const limit);
int check_status(const char * cursor, const char * const limit);
int check_type(const char * cursor, const char * const limit);
int check_int(const char * cursor, const char * const limit);

int check_get_subitem(const char * cursor, const char * const limit);

int parse_get_url(const char * cursor, const char * const limit,request *r);
int parse_put_url(const char * cursor, const char * const limit,request *r);

int check_date(const char * cursor, const char * const limit);
int validate_json(jsonEntry *je,const char*json,int validate);
void print_json(jsonEntry *e,const char*js);
void clear_json(jsonEntry *e);

int analise_json(const char *js,int validate);
int handle_request(redisContext *c,const char*method,const char*url,const char*body,int validate,char*result);

