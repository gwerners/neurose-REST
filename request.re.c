#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <time.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include "request.h"
#include "jsmn.h"


#include <uuid/uuid.h>


//DEFINES
#define YYCTYPE        char
#define YYPEEK()       (cursor >= limit ? 0 : *cursor)
#define YYSKIP()       ++cursor
#define YYBACKUP()     marker = cursor
#define YYBACKUPCTX()  ctxmarker = cursor
#define YYRESTORE()    cursor = marker
#define YYRESTORECTX() cursor = ctxmarker

#define prn_entry(NAME)\
    printf("%s=%*.*s\n",#NAME,0,e->NAME.end-e->NAME.start,js+e->NAME.start);
#define prn_item(NAME)\
    printf("\t%s=%*.*s\n",#NAME,0,il->i.NAME.end-il->i.NAME.start,js+il->i.NAME.start);
#define prn_transaction(NAME)\
    printf("\t%s=%*.*s\n",#NAME,0,tl->t.NAME.end-tl->t.NAME.start,js+tl->t.NAME.start);
	
const char* gwf_json=
"{   \"id\" : \"b8d36032-42d9-11e6-9d7c-f04da2e5162d\","
"   \"number\" : \"1\","
"   \"reference\" : \"x\","
"   \"status\" : \"CLOSED\","
"   \"created_at\" : \"2016-07-05 15:40:14.010\","
"   \"updated_at\" : \"2016-07-05T15:40:14.055Z\","
"   \"items\" : ["
"      { "
"         \"sku\" : \"b8d36032-42d9-11e6-9d7c-f04da2e5162d\","
"         \"unit_price\" : 1000,"
"         \"quantity\" : 10"
"      }  "
"   ] ,"
"   \"notes\" : \"\","
"   \"transactions\" : ["
"      { "
"         \"id\" : \"b8d36032-42d9-11e6-9d7c-f04da2e5162d\","
"         \"external_id\" : \"sbrubles\","
"         \"amount\" : 1000,"
"         \"type\" : \"PAYMENT|CANCEL\","
"         \"authorization_code\" : \"94721719\","
"         \"card_brand\" : \"AMEX\" ,"
"         \"card_bin\" : \"710\","
"         \"card_last\" : \"4749\""
"      } "
"   ],"
"   \"price\" : 1000}"
;

#define iNC(VLR)do{++VLR;if(VLR>=r)return JSMN_ERROR_INVAL;}while(0);

int gen_uuid(char*out)
{

  uuid_t uuid;

  if(out)
  {
    // generate
#if defined(__APPLE__)
    uuid_generate_time(uuid);
#else
    uuid_generate_time_safe(uuid);
#endif
    //"1b4e28ba-2fa1-11d2-883f-0016d3cca427" + "\0"
    uuid_unparse_lower(uuid, out);
    IFDBG printf("gen_uuid uuid=%s\n", out);
    strcpy(out,(char*)out);
    return strlen((char*)out);
  }
  return 0;
}
int parse_method(const char * cursor, const char * const limit)
{
   const char * marker;
   int id=0;
   /*!re2c
      re2c:yyfill:enable = 0;
      CR             = '\r';
      LF             = '\n';
      SP             = ' ';
      HT             = [\011];
      CRLF           = CR LF;
      digit         = [0-9];
      hex           = digit | [A-F] | [a-f];
      escaped       = "%" hex hex;
      reserved      = ";" | "/" | "?" | ":" | "@" | "&" | "=" | "+" | "$";
      mark          = "-" | "_" | "." | "!" | "~" | "*" | "'" | "(" | ")" ;
      lowalpha      = [a-z];
      upalpha       = [A-Z];
      alpha         = lowalpha | upalpha;
      alphanum      = alpha | digit;
      any           = alphanum | mark | reserved;
      part1 = hex hex hex hex hex hex hex hex;
      part2 = hex hex hex hex;
      part3 = hex hex hex hex hex hex hex hex hex hex hex hex;
    */
   /*!re2c
         ""               { id=0 ;goto lbl_end;}
         "GET"            { id=GET_METHOD;goto lbl_end;}
         "POST"           { id=POST_METHOD;goto lbl_end;}
         "PUT"            { id=PUT_METHOD;goto lbl_end;}
         "DELETE"         { id=DELETE_METHOD;goto lbl_end;}
   */
lbl_end:
   return id;
}

int check_uuid(const char * cursor, const char * const limit)
{
   const char * marker;
   int id=0;
   /*!re2c

        ""            { id=0; goto lbl_end;}
        part1 "-" part2 "-" part2 "-" part2 "-" part3 { return 36;}
   */
lbl_end:
   return id;
}

int check_get_subitem(const char * cursor, const char * const limit)
{
   const char * marker;
   int id=0;
   /*!re2c

        ""             { id=0; goto lbl_end;}
        "items"        { return GET_ITEMS;}
        "transactions" { return GET_TRANSACTIONS;}
   */
lbl_end:
   return id;
}

int check_put_subitem(const char * cursor, const char * const limit)
{
   const char * marker;
   int id=0;
   /*!re2c

        ""             { id=0; goto lbl_end;}
        "transactions" { return PUT_TRANSACTIONS;}
   */
lbl_end:
   return id;
}

int parse_get_url(const char * cursor, const char * const limit,request *r)
{
    const char * marker=cursor;
    int id=0,ret;
    /*!re2c

    property = "number" | "reference" | "status" | "created_at" | "updated_at" | "notes" | "price";
    ""  { id=0 ;goto lbl_end;}
    "/orders?" {r->id.start=marker-cursor; id=GET_BY_VALUE;goto lbl_end;}
    "/orders/" {
        r->id.start=cursor-marker;
        marker=cursor;
        ret=check_uuid(cursor,limit);
        if(ret==36)
        {
            r->id.end=r->id.start+36;
            cursor+=36;
            IFDBG printf("GET [%d]found UUID[%*.*s]\n",ret,0,r->id.end-r->id.start,marker);
            if(*cursor=='/')
            {
                ++cursor;
            }
            ret=check_get_subitem(cursor,limit);
            IFDBG printf("%d cursor[%s]\n",ret,cursor);
            switch(ret)
            {
                case GET_ITEMS:
					IFDBG printf("found get items\n");
					return GET_ITEMS;
                case GET_TRANSACTIONS:
					IFDBG printf("found get transactions\n");
					return GET_TRANSACTIONS;
                default:
                    IFDBG printf("default get sub item\n");
                    break;
            }
            return GET_BY_ID;
        }else{
            r->id.end=(cursor-marker)+36;
            IFDBG printf("[%d]NOT found UUID[%*.*s]\n",ret,0,r->id.end-r->id.start,marker);
        }
        goto lbl_end;
    }
	"/orders" {id=GET_ALL;}
    */
lbl_end:
   return id;
}

int parse_put_url(const char * cursor, const char * const limit,request *r)
{
    const char * marker=cursor;
    int id=0,ret;
    /*!re2c
    ""  { id=0 ;goto lbl_end;}
    "/orders/" {
        r->id.start=cursor-marker;
        marker=cursor;
        ret=check_uuid(cursor,limit);
        if(ret==36)
        {
            r->id.end=r->id.start+36;
            cursor+=36;
            IFDBG printf("PUT [%d]found ID UUID [%*.*s]\n",ret,0,r->id.end-r->id.start,marker);
            if(*cursor=='/')
            {
                ++cursor;
            }
            ret=check_put_subitem(cursor,limit);
            IFDBG printf("%d cursor[%s]\n",ret,cursor);
            switch(ret)
            {
                case PUT_TRANSACTIONS:
					IFDBG printf("found put transactions\n");
					cursor+=13;// advance "transactions/"
			        r->id.start=(cursor-marker);
			        marker=cursor;
			        ret=check_uuid(cursor,limit);
			        if(ret==36)
					{
			            r->tid.end=r->tid.start+36;
			            cursor+=36;
			            IFDBG printf("PUT [%d]found TID UUID[%*.*s]\n",ret,0,r->tid.end-r->tid.start,marker);
					}else{
						IFDBG printf("[%d]error processing put transaction id[%s]\n",ret,cursor+13);
					}
					return PUT_TRANSACTIONS;
                default:
                    break;
            }
            return PUT_BY_ID;
        }else{
            r->id.end=(cursor-marker)+36;
            IFDBG printf("[%d]NOT found UUID[%*.*s]\n",ret,0,r->id.end-r->id.start,marker);
        }
        goto lbl_end;
    }
	"/orders" {id=PUT_ALL;}
    */
lbl_end:
   return id;
}

int parse_items(const char * cursor, const char * const limit)
{
   const char * marker;
   int id=0;
   /*!re2c
      ""            { id=0 ;goto lbl_end;}
      "sku"         { id=11;goto lbl_end;}
      "unit_price"  { id=12;goto lbl_end;}
      "quantity"    { id=13;goto lbl_end;}
   */
lbl_end:
   return id;
}

int parse_transactions(const char * cursor, const char * const limit)
{
   const char * marker;
   int id=0;
   /*!re2c
         ""                  { id=0 ;goto lbl_end;}
         "id"                { id=14;goto lbl_end;}
         "external_id"       { id=15;goto lbl_end;}
         "amount"            { id=16;goto lbl_end;}
         "type"              { id=17;goto lbl_end;}
         "authorization_code"{ id=18;goto lbl_end;}
         "card_brand"        { id=19;goto lbl_end;}
         "card_bin"          { id=20;goto lbl_end;}
         "card_last"         { id=21;goto lbl_end;}
   */
lbl_end:
   return id;
}

int parse_id(const char * cursor, const char * const limit)
{
   const char * marker;
   int id=0;

   /*!re2c
      ""            { id=0; goto lbl_end;}
      "id"          { id=1; goto lbl_end;}
      "number"      { id=2; goto lbl_end;}
      "reference"   { id=3; goto lbl_end;}
      "status"      { id=4; goto lbl_end;}
      "created_at"  { id=5; goto lbl_end;}
      "updated_at"  { id=6; goto lbl_end;}
      "items"       { id=7; goto lbl_end;}
      "notes"       { id=8; goto lbl_end;}
      "transactions"{ id=9; goto lbl_end;}
      "price"       { id=10;goto lbl_end;}
   */
lbl_end:
   return id;
}

int check_status(const char * cursor, const char * const limit)
{
   const char * marker;
   int id=0;
   /*!re2c

      ""          { id=0; goto lbl_end;}
      "DRAFT"     { id=1; goto lbl_end;}
      "ENTERED"   { id=1<<2; goto lbl_end;}
      "CANCELED"  { id=1<<3; goto lbl_end;}
      "PAID"      { id=1<<4; goto lbl_end;}
      "APPROVED"  { id=1<<5; goto lbl_end;}
      "REJECTED"  { id=1<<6; goto lbl_end;}
      "RE-ENTERED"{ id=1<<7; goto lbl_end;}
      "CLOSED"    { id=1<<8; goto lbl_end;}
   */
lbl_end:
   return id;
}

int check_type(const char * cursor, const char * const limit)
{
   const char * marker;
   int id=0;
   /*!re2c

      ""        { id=0; goto lbl_end;}
      "PAYMENT" { id=1; goto lbl_end;}
      "CANCEL"  { id=1<<2; goto lbl_end;}
   */
lbl_end:
   return id;
}

int check_int(const char * cursor, const char * const limit)
{
   int id=0;
   /*!re2c

      ""        { id=0; goto lbl_end;}
      "0" digit+{ id=0; goto lbl_end;}
      digit+    { id=1; goto lbl_end;}
      any+      { id=0; goto lbl_end;}
   */
lbl_end:
   return id;
}

int check_date(const char * cursor, const char * const limit)
{
   const char * marker;
   int id=0;
   /*!re2c
      DATE = digit digit digit digit '-' digit digit '-' digit digit;
      TIME = digit digit ':' digit digit ':' digit digit;
      DATE_END = '.' digit digit digit;
      ""        { id=0; goto lbl_end;}
      DATE 'T' TIME DATE_END 'Z'  { id=1; goto lbl_end;}
      DATE ' ' TIME DATE_END  { id=1; goto lbl_end;}
   */
lbl_end:
   return id;
}


int validate_json(jsonEntry *je,const char*json,int validate)
{
    int r,i,id=0,size,items,entries,ret;
    jsmn_parser p;
    jsmntok_t tokens[MAX_TOKENS];
    item_list *il;
    transaction_list *tl;
    memset(&tokens,0,sizeof tokens);
    jsmn_init(&p);
    r = jsmn_parse(&p, json, strlen(json), tokens, MAX_TOKENS);
    switch(r)
    {
        case JSMN_ERROR_NOMEM: 
			IFDBG printf("parser ERROR:Not enough tokens were provided\n");
            return JSMN_ERROR_NOMEM;
        case JSMN_ERROR_INVAL: 
			IFDBG printf("Invalid character inside JSON string\n");
            return JSMN_ERROR_INVAL;
        case JSMN_ERROR_PART : 
			IFDBG printf("The string is not a full JSON packet, more bytes expected\n");
            return JSMN_ERROR_PART;
    }
    //r == numero de tokens parseados
    i=0;
    while(i<r)
    {
        size=(tokens[i]).size;
        IFDBG printf(".tokens[%d]child[%d]=%*.*s\n",i,(tokens[i]).size,0,(tokens[i]).end-(tokens[i]).start,json+(tokens[i]).start);

        iNC(i);
        while(size--)
        {
            id=parse_id((const char*)&json[tokens[i].start],(const char*)&json[tokens[i].end]);
            IFDBG printf("token id[%d]%*.*s\n",id,0,(tokens[i]).end-(tokens[i]).start,json+(tokens[i]).start);
            switch(id)
            {
                case ID          :
                    IFDBG printf("match ID.\n");
                    iNC(i);
                    if(validate && !check_uuid((const char*)&json[tokens[i].start],(const char*)&json[tokens[i].end]))
                    {
                        IFDBG printf("invalid UUID.\n");
                        return -2;
                    }
                    je->id.start=tokens[i].start;
                    je->id.end=tokens[i].end;
                    iNC(i);
                    break;
                case NUMBER      :
                    IFDBG printf("match NUMBER.\n");
                    iNC(i);
                    je->number.start=tokens[i].start;
                    je->number.end=tokens[i].end;
                    iNC(i);
                    break;
                case REFERENCE   :
                    IFDBG printf("match REFERENCE.\n");
                    iNC(i);
                    je->reference.start=tokens[i].start;
                    je->reference.end=tokens[i].end;
                    iNC(i);
                    break;
                case STATUS      :
                    IFDBG printf("match STATUS.\n");
                    iNC(i);
                    if(validate && !check_status((const char*)&json[tokens[i].start],(const char*)&json[tokens[i].end]))
                    {
                        IFDBG printf("invalid STATUS.\n");
                        return -2;
                    }
                    je->status.start=tokens[i].start;
                    je->status.end=tokens[i].end;
                    iNC(i);
                    break;
                case CREATED_AT  :
                    IFDBG printf("match CREATED_AT.\n");
                    iNC(i);
                    if(validate && !check_date((const char*)&json[tokens[i].start],(const char*)&json[tokens[i].end]))
                    {
                        IFDBG printf("invalid CREATED_AT.\n");
                        return -2;
                    }
                    je->created_at.start=tokens[i].start;
                    je->created_at.end=tokens[i].end;
                    iNC(i);
                    break;
                case UPDATED_AT  :
                    IFDBG printf("match UPDATED_AT.\n");
                    iNC(i);
                    if(validate && !check_date((const char*)&json[tokens[i].start],(const char*)&json[tokens[i].end]))
                    {
                        IFDBG printf("invalid UPDATED_AT.\n");
                        return -2;
                    }
                    je->updated_at.start=tokens[i].start;
                    je->updated_at.end=tokens[i].end;
                    iNC(i);
                    break;
                case ITEMS       :
                    IFDBG printf("match ITEMS:\n");
                    iNC(i);
                    items=(tokens[i]).size;
                    IFDBG printf("ARRAY[%d]\n",items);
                    iNC(i);

                    while(items)
                    {
                        il=(item_list*)calloc(1,sizeof(item_list));
                        if(je->items==NULL)
                        {
                            je->items=il;
                        }else{
                            il->next = je->items;
                            je->items = il;
                        }
                        IFDBG printf("ITEMS[%d]\n",items);
                        IFDBG printf("1 tokens[%d]child[%d]=%*.*s\n",i,(tokens[i]).size,0,(tokens[i]).end-(tokens[i]).start,json+(tokens[i]).start);
                        entries=(tokens[i]).size;
                        iNC(i);
                        IFDBG printf("3 tokens[%d]child[%d]=%*.*s\n",i,(tokens[i]).size,0,(tokens[i]).end-(tokens[i]).start,json+(tokens[i]).start);

                        while(entries)
                        {
                            IFDBG printf("ENTRY[%d]\n",entries);
                            ret=parse_items((const char*)&json[tokens[i].start],(const char*)&json[tokens[i].end]);
                            switch(ret)
                            {
                                case SKU        :
                                    IFDBG printf("match ITEM SKU.\n");
                                    IFDBG printf(".tokens[%d]child[%d]=%*.*s\n",i,(tokens[i]).size,0,(tokens[i]).end-(tokens[i]).start,json+(tokens[i]).start);
                                    iNC(i);
                                    if(validate && !check_uuid((const char*)&json[tokens[i].start],(const char*)&json[tokens[i].end]))
                                    {
                                        IFDBG printf("invalid UUID on ITEM SKU.\n");
                                        return -2;
                                    }
                                    il->i.sku.start=tokens[i].start;
                                    il->i.sku.end=tokens[i].end;
                                    IFDBG printf(".tokens[%d]child[%d]=%*.*s\n",i,(tokens[i]).size,0,(tokens[i]).end-(tokens[i]).start,json+(tokens[i]).start);
                                    iNC(i);
                                    break;
                                case UNIT_PRICE :
                                    IFDBG printf("match ITEM UNIT_PRICE.\n");
                                    IFDBG printf(".tokens[%d]child[%d]=%*.*s\n",i,(tokens[i]).size,0,(tokens[i]).end-(tokens[i]).start,json+(tokens[i]).start);
                                    iNC(i);
                                    if(validate && !check_int((const char*)&json[tokens[i].start],(const char*)&json[tokens[i].end]))
                                    {
                                        IFDBG printf("invalid INTEGER on ITEM UNIT_PRICE.\n");
                                        return -2;
                                    }
                                    il->i.unit_price.start=tokens[i].start;
                                    il->i.unit_price.end=tokens[i].end;
                                    IFDBG printf(".tokens[%d]child[%d]=%*.*s\n",i,(tokens[i]).size,0,(tokens[i]).end-(tokens[i]).start,json+(tokens[i]).start);
                                    iNC(i);
                                    break;
                                case QUANTITY   :
                                    IFDBG printf("match ITEM QUANTITY.\n");
                                    IFDBG printf(".tokens[%d]child[%d]=%*.*s\n",i,(tokens[i]).size,0,(tokens[i]).end-(tokens[i]).start,json+(tokens[i]).start);
                                    iNC(i);
                                    if(validate && !check_int((const char*)&json[tokens[i].start],(const char*)&json[tokens[i].end]))
                                    {
                                        IFDBG printf("invalid INTEGER on ITEM QUANTITY.\n");
                                        return -2;
                                    }
                                    il->i.quantity.start=tokens[i].start;
                                    il->i.quantity.end=tokens[i].end;
                                    IFDBG printf(".tokens[%d]child[%d]=%*.*s\n",i,(tokens[i]).size,0,(tokens[i]).end-(tokens[i]).start,json+(tokens[i]).start);
                                    iNC(i);
                                    break;
                                default:
                                    IFDBG printf("INVALID ENTRY[%d]\n",entries);
                                    return -2;
                            }
                            --entries;
                            IFDBG printf("--e tokens[%d]child[%d]=%*.*s\n",i,(tokens[i]).size,0,(tokens[i]).end-(tokens[i]).start,json+(tokens[i]).start);
                        }
                        --items;
                        IFDBG printf("--i tokens[%d]child[%d]=%*.*s\n",i,(tokens[i]).size,0,(tokens[i]).end-(tokens[i]).start,json+(tokens[i]).start);
                    }
                    break;
                case NOTES       :
                    IFDBG printf("match NOTES.\n");
                    iNC(i);
                    je->notes.start=tokens[i].start;
                    je->notes.end=tokens[i].end;
                    iNC(i);
                    break;
                case TRANSACTIONS:
                    IFDBG printf("match ITEMS:\n");
                    iNC(i);
                    items=(tokens[i]).size;
                    IFDBG printf("ARRAY[%d]\n",items);
                    iNC(i);
                    while(items)
                    {
                        IFDBG printf("ITEMS[%d]\n",items);
                        IFDBG printf("1 tokens[%d]child[%d]=%*.*s\n",i,(tokens[i]).size,0,(tokens[i]).end-(tokens[i]).start,json+(tokens[i]).start);
                        entries=(tokens[i]).size;
                        iNC(i);
                        IFDBG printf("2 tokens[%d]child[%d]=%*.*s\n",i,(tokens[i]).size,0,(tokens[i]).end-(tokens[i]).start,json+(tokens[i]).start);
                        tl=(transaction_list*)calloc(1,sizeof(transaction_list));
                        if(je->transactions==NULL)
                        {
                            je->transactions=tl;
                        }else{
                            tl->next = je->transactions;
                            je->transactions = tl;
                        }
                        while(entries)
                        {
                            IFDBG printf("ENTRY[%d]\n",entries);
                            ret=parse_transactions((const char*)&json[tokens[i].start],(const char*)&json[tokens[i].end]);
                            IFDBG printf("3 tokens[%d]child[%d]=%*.*s\n",i,(tokens[i]).size,0,(tokens[i]).end-(tokens[i]).start,json+(tokens[i]).start);
                            switch(ret)
                            {
                                case TID :
                                    IFDBG printf("match TRANSACTION ID.\n");
                                    IFDBG printf(".tokens[%d]child[%d]=%*.*s\n",i,(tokens[i]).size,0,(tokens[i]).end-(tokens[i]).start,json+(tokens[i]).start);
                                    iNC(i);
                                    if(validate && !check_uuid((const char*)&json[tokens[i].start],(const char*)&json[tokens[i].end]))
                                    {
                                        IFDBG printf("invalid UUID on TRANSACTION ID.\n");
                                        return -2;
                                    }
                                    tl->t.tid.start=tokens[i].start;
                                    tl->t.tid.end=tokens[i].end;
                                    IFDBG printf(".tokens[%d]child[%d]=%*.*s\n",i,(tokens[i]).size,0,(tokens[i]).end-(tokens[i]).start,json+(tokens[i]).start);
                                    iNC(i);
                                    break;
                                case EXTERNAL_ID :
                                    IFDBG printf("match TRANSACTION EXTERNAL_ID.\n");
                                    IFDBG printf(".tokens[%d]child[%d]=%*.*s\n",i,(tokens[i]).size,0,(tokens[i]).end-(tokens[i]).start,json+(tokens[i]).start);
                                    iNC(i);
                                    //check here!
                                    tl->t.external_id.start=tokens[i].start;
                                    tl->t.external_id.end=tokens[i].end;
                                    iNC(i);
                                    break;
                                case AMOUNT   :
                                    IFDBG printf("match TRANSACTION AMOUNT.\n");
                                    IFDBG printf(".tokens[%d]child[%d]=%*.*s\n",i,(tokens[i]).size,0,(tokens[i]).end-(tokens[i]).start,json+(tokens[i]).start);
                                    iNC(i);
                                    if(validate && !check_int((const char*)&json[tokens[i].start],(const char*)&json[tokens[i].end]))
                                    {
                                        IFDBG printf("invalid INTEGER on TRANSACTION AMOUNT.\n");
                                        return -2;
                                    }
                                    tl->t.amount.start=tokens[i].start;
                                    tl->t.amount.end=tokens[i].end;
                                    IFDBG printf(".tokens[%d]child[%d]=%*.*s\n",i,(tokens[i]).size,0,(tokens[i]).end-(tokens[i]).start,json+(tokens[i]).start);
                                    iNC(i);
                                    break;
                                case TYPE :
                                    IFDBG printf("match TRANSACTION TYPE.\n");
                                    IFDBG printf(".tokens[%d]child[%d]=%*.*s\n",i,(tokens[i]).size,0,(tokens[i]).end-(tokens[i]).start,json+(tokens[i]).start);
                                    iNC(i);
                                    //check here!
                                    tl->t.type.start=tokens[i].start;
                                    tl->t.type.end=tokens[i].end;
                                    iNC(i);
                                    break;
                                case AUTHORIZATION_CODE :
                                    IFDBG printf("match TRANSACTION AUTHORIZATION_CODE.\n");
                                    IFDBG printf(".tokens[%d]child[%d]=%*.*s\n",i,(tokens[i]).size,0,(tokens[i]).end-(tokens[i]).start,json+(tokens[i]).start);
                                    iNC(i);
                                    //check here!
                                    tl->t.authorization_code.start=tokens[i].start;
                                    tl->t.authorization_code.end=tokens[i].end;
                                    iNC(i);
                                    break;
                                case CARD_BRAND :
                                    IFDBG printf("match TRANSACTION CARD_BRAND.\n");
                                    IFDBG printf(".tokens[%d]child[%d]=%*.*s\n",i,(tokens[i]).size,0,(tokens[i]).end-(tokens[i]).start,json+(tokens[i]).start);
                                    iNC(i);
                                    //check here!
                                    tl->t.card_brand.start=tokens[i].start;
                                    tl->t.card_brand.end=tokens[i].end;
                                    iNC(i);
                                    break;
                                case CARD_BIN :
                                    IFDBG printf("match TRANSACTION CARD_BIN.\n");
                                    IFDBG printf(".tokens[%d]child[%d]=%*.*s\n",i,(tokens[i]).size,0,(tokens[i]).end-(tokens[i]).start,json+(tokens[i]).start);
                                    iNC(i);
                                    //check here!
                                    tl->t.card_bin.start=tokens[i].start;
                                    tl->t.card_bin.end=tokens[i].end;
                                    iNC(i);
                                    break;
                                case CARD_LAST :
                                    IFDBG printf("match TRANSACTION CARD_LAST.\n");
                                    IFDBG printf(".tokens[%d]child[%d]=%*.*s\n",i,(tokens[i]).size,0,(tokens[i]).end-(tokens[i]).start,json+(tokens[i]).start);
                                    iNC(i);
                                    //check here!
                                    tl->t.card_last.start=tokens[i].start;
                                    tl->t.card_last.end=tokens[i].end;
                                    iNC(i);
                                    break;
                                default:
                                    IFDBG printf("INVALID ENTRY[%d]\n",entries);
                                    return -2;
                            }
                            --entries;
                            IFDBG printf("--e tokens[%d]child[%d]=%*.*s\n",i,(tokens[i]).size,0,(tokens[i]).end-(tokens[i]).start,json+(tokens[i]).start);
                        }
                        --items;
                        IFDBG printf("--i tokens[%d]child[%d]=%*.*s\n",i,(tokens[i]).size,0,(tokens[i]).end-(tokens[i]).start,json+(tokens[i]).start);
                    }
                    break;
                case PRICE       :
                    IFDBG printf("match PRICE.\n");
                    iNC(i);
                    if(validate && !check_int((const char*)&json[tokens[i].start],(const char*)&json[tokens[i].end]))
                    {
                        IFDBG printf("invalid PRICE.\n");
                        return -2;
                    }
                    je->price.start=tokens[i].start;
                    je->price.end=tokens[i].end;
                    iNC(i);
                    break;
                default:
                    IFDBG printf("NO match.\n");
                    IFDBG printf("tokens[%d]child[%d]=%*.*s\n",i,(tokens[i]).size,0,(tokens[i]).end-(tokens[i]).start,json+(tokens[i]).start);
                    return -2;
            }
        }
    }
   return 0;
}

void print_json(jsonEntry *e,const char*js)
{
    item_list *il;
    transaction_list *tl;
    prn_entry(id)
    prn_entry(number)
    prn_entry(reference)
    prn_entry(status)
    prn_entry(created_at)
    prn_entry(updated_at)
    //items
    il=e->items;
    while(il!=NULL)
    {
        printf("item=\n");
        prn_item(sku)
        prn_item(unit_price)
        prn_item(quantity)
        il=il->next;
    }
    prn_entry(notes)
    //transactions
    tl=e->transactions;
    while(tl!=NULL)
    {
        printf("transaction=\n");
        prn_transaction(tid)
        prn_transaction(external_id);
        prn_transaction(amount);
        prn_transaction(type);
        prn_transaction(authorization_code);
        prn_transaction(card_brand);
        prn_transaction(card_bin);
        prn_transaction(card_last);
        tl=tl->next;
    }
    prn_entry(price)
}

void clear_json(jsonEntry *e)
{
    item_list *il;
    transaction_list *tl;
    il=e->items;
    while(e->items!=NULL)
    {
        il=e->items->next;
        free(e->items);
        e->items=il;
    }
    while(e->transactions!=NULL)
    {
        tl=e->transactions->next;
        free(e->transactions);
        e->transactions=tl;
    }
    memset(e,0,sizeof(jsonEntry));
}

int analise_json(redisContext *c,jsonEntry *je,const char *js,int validate)
{
	int r;
	jsmn_parser p;
	jsmntok_t tokens[MAX_TOKENS];
	memset(&tokens,0,sizeof tokens);
	jsmn_init(&p);
	r = jsmn_parse(&p, js, strlen(js), tokens, MAX_TOKENS);
	switch(r)
	{
		case JSMN_ERROR_NOMEM: 
			IFDBG printf("parser ERROR:tokens insuficientes\n");
			return JSMN_ERROR_NOMEM;
		case JSMN_ERROR_INVAL: 
			IFDBG printf("Caracter invalido dentro da JSON string\n");
			return JSMN_ERROR_INVAL;
		case JSMN_ERROR_PART : 
			IFDBG printf("A string nao eh um pacote JSON completo, mais bytes esperados!\n");
			IFDBG printf("json[%s]\n",js);
			return JSMN_ERROR_PART;
	}	
	//r == numero de tokens parseados
	memset(je,0,sizeof(jsonEntry));
	validate_json(je,js,validate);
	//print_json(je,js);
	
	return JSMN_SUCCESS;
}


void appendJsonElementStr(char* json, char* elementName, char* element, int* jsonLen)
{
  if(*jsonLen != 0)
  {
    char last = json[*jsonLen-1];
    if(last != '{' && last != '[')
    {
      strcat(json+(*jsonLen), ",");
      (*jsonLen)++;
    }
  }
  strcat(json+(*jsonLen), "\"");
  strcat(json+(*jsonLen), elementName);
  strcat(json+(*jsonLen), "\":\"");
  strcat(json+(*jsonLen), element);
  strcat(json+(*jsonLen), "\"");
  (*jsonLen) += (strlen(elementName)+strlen(element)+5);
}

void openJsonObj(char* json, char* objName, int* jsonLen)
{
  if(*jsonLen != 0)
  {
    char last = json[(*jsonLen)-1];
    if(last != '{' && last != '[')
    {
      strcat(json+(*jsonLen), ",");
      (*jsonLen)++; 
    }
  }
  if(objName != NULL)
  {
    strcat(json+(*jsonLen), "\"");
    strcat(json+(*jsonLen), objName);
    strcat(json+(*jsonLen), "\":");
    (*jsonLen)+=(strlen(objName)+3);
  }
  strcat(json+(*jsonLen), "{");
  (*jsonLen)++;
}

void closeJsonObj(char* json, int*jsonLen)
{
  strcat(json+(*jsonLen), "}");
  (*jsonLen)++;
}

void openJsonArray(char* json, char* arrayName, int*jsonLen)
{
  strcat(json+(*jsonLen), "\"");
  strcat(json+(*jsonLen), arrayName);
  strcat(json+(*jsonLen), "\":[");
  (*jsonLen)+=(strlen(arrayName)+4);
}

void closeJsonArray(char* json, int*jsonLen)
{
  strcat(json+(*jsonLen), "]");
  (*jsonLen)++;
}

void get_by_id(redisContext *c, char*json, char* id, int*jsonLen)
{
    unsigned int j;
    redisReply *reply;
  
    printf("get_by_id request %s\n",id);
    reply = (redisReply *)redisCommand(c,"HGETALL %s",id);
    for (j = 1; j < reply->elements; j+=2) {
        printf("%u)REDIS_REPLY_ARRAY %s\n", j+1, reply->element[j]->str);
        appendJsonElementStr(json, reply->element[j-1]->str, reply->element[j]->str,jsonLen);
    }
    //printf("\n%s\n",json);
    freeReplyObject(reply);
}

void get_by_id(redisContext *c, char*json, char* id)
{
  int jsonLen=0;
  get_by_id(c,json,id,&jsonLen);
}

void process_order_id(redisContext *c, redisReply *reply, char*json,int*jsonLen)
{
  openJsonObj(json, NULL, jsonLen);
  openJsonArray(json, "orders", jsonLen);
  for (int j = 0; j < reply->elements; j++) {
      openJsonObj(json, NULL, jsonLen);
      printf("\t.%u)REDIS_REPLY_ARRAY %s\n", j+1, reply->element[j]->str);
      appendJsonElementStr(json, "id", 6+reply->element[j]->str, jsonLen);
      get_by_id(c,json, reply->element[j]->str, jsonLen);
      closeJsonObj(json, jsonLen);
  }
  closeJsonArray(json, jsonLen);
  closeJsonObj(json, jsonLen);
  //printf("\n%s\n", json);
}

void get_all(redisContext *c, char*result)
{
	unsigned int j;
	char command[128];
	unsigned int cursor = 0;
	redisReply *reply;
	int jsonLen = 0;


  do
  {
    sprintf(command, "SCAN %d MATCH order:*", cursor);
    reply = (redisReply *)redisCommand(c,command);
    cursor = atoi(reply->element[0]->str);
    
    for (j = 1; j < reply->elements; j++)
    {
	  printf("%u)REDIS_REPLY_ARRAY %s\n", j+1, reply->element[j]->str);
      process_order_id(c, reply->element[j], result, &jsonLen);
    }

  } while(cursor != 0);

  printf("\n%s\n",result);
  freeReplyObject(reply);
	
}
void get_by_value(redisContext *c,char*result)
{
	unsigned int j;
	char command[128];
	unsigned int cursor = 0;
	redisReply *reply;
	int jsonLen = 0;
  do
  {
    sprintf(command, "SCAN %d MATCH order:*", cursor);
    reply = (redisReply *)redisCommand(c,command);
	if(reply)
	{
    cursor = reply->element[0]->str[0] - '0';
    for (j = 1; j < reply->elements; j++)
    {
		printf("%u)REDIS_REPLY_ARRAY %s\n", j+1, reply->element[j]->str);
		process_order_id(c, reply->element[j], result, &jsonLen);
    }
	}

  } while(cursor != 0);

  
  freeReplyObject(reply);
}

void get_items(redisContext *c)
{
	unsigned int j;
	redisReply *reply;
	//GET /orders/b8d36032-42d9-11e6-9d7c-f04da2e5162d/items

	//LLEN order:b8d36032-42d9-11e6-9d7c-f04da2e5162d.items
	//LRANGE order:b8d36032-42d9-11e6-9d7c-f04da2e5162d.items 0 %len%-1
	
    reply = (redisReply *)redisCommand(c,"SCAN 0 MATCH order:*");
    if (reply->type == REDIS_REPLY_ARRAY) {
        for (j = 0; j < reply->elements; j++) {
            printf("%u) %s\n", j, reply->element[j]->str);
        }
    }
    freeReplyObject(reply);
}
void get_transactions(redisContext *c)
{
	unsigned int j;
	redisReply *reply;
	//GET /orders/b8d36032-42d9-11e6-9d7c-f04da2e5162d/transactions

	//LLEN order:b8d36032-42d9-11e6-9d7c-f04da2e5162d.transactions
	//LRANGE order:b8d36032-42d9-11e6-9d7c-f04da2e5162d.transactions 0 %len%-1
    reply = (redisReply *)redisCommand(c,"SCAN 0 MATCH order:*");
    if (reply->type == REDIS_REPLY_ARRAY) {
        for (j = 0; j < reply->elements; j++) {
            printf("%u) %s\n", j, reply->element[j]->str);
        }
    }
    freeReplyObject(reply);
}
void post(redisContext *c,jsonEntry *je)
{
    int ret;
    char uuid_str[37];
	  redisReply *reply;
	  ret=gen_uuid((char*)uuid_str);
	  if(!ret)
	  {
	    printf("post failed to generate UUID!\n");
	  }else{
	    IFDBG printf("post new UUID [%s]\n",(char*)uuid_str);
	  }
    reply = (redisReply *)redisCommand(c,"MULTI");
	printf("MULTI reply %s\n", reply->str);
    freeReplyObject(reply);
	
	//HMSET order:b8d36032-42d9-11e6-9d7c-f04da2e5162d status DRAFT created_at 35468461516514 updated_at 35468461516514 price 1000
    reply = (redisReply *)redisCommand(c,"HMSET order:%s status %s created_at %s updated_at %s price %s"
		,(char*)uuid_str//"b8d36032-42d9-11e6-9d7c-f04da2e5162d"
		,"DRAFT"
		,"35468461516514"
		,"35468461516514"
		,"1000");
	printf("HMSET reply %s\n", reply->str);
    freeReplyObject(reply);
	
	//RPUSH order:b8d36032-42d9-11e6-9d7c-f04da2e5162d.items dfg9087er908t7 100 3 |
    reply = (redisReply *)redisCommand(c,"RPUSH order:%s.items %s %s %s |"
		,(char*)uuid_str//"b8d36032-42d9-11e6-9d7c-f04da2e5162d"
		,"dfg9087er908t7"
		,"100"
		,"3");
	printf("RPUSH reply %s\n", reply->str);
    freeReplyObject(reply);
	
	/*
	"         \"id\" : \"b8d36032-42d9-11e6-9d7c-f04da2e5162d\"," +
	"         \"external_id\" : \"sbrubles\"," +
	"         \"amount\" : 1000," +
	"         \"type\" : \"PAYMENT|CANCEL\"," +
	"         \"authorization_code\" : \"94721719\"," +
	"         \"card_brand\" : \"AMEX\" ," +
	"         \"card_bin\" : \"710\"," +
	"         \"card_last\" : \"4749\"" +
	"      } " +
	*/
	//RPUSH order:b8d36032-42d9-11e6-9d7c-f04da2e5162d.transactions ... |
    reply = (redisReply *)redisCommand(c,"RPUSH order:%s.transactions id %s external_id %s|"
		,(char*)uuid_str//"b8d36032-42d9-11e6-9d7c-f04da2e5162d"
		,"b8d36032-42d9-11e6-9d7c-f04da2e5162c"
		, "sbrubles");
	printf("RPUSH reply %s\n", reply->str);
    freeReplyObject(reply);
	
  reply = (redisReply *)redisCommand(c,"EXEC");
	printf("EXEC reply %s\n", reply->str);
  freeReplyObject(reply);
	
}




void delete_all(redisContext *c)
{
	redisReply *reply;
	//DELETE /orders
    reply = (redisReply *)redisCommand(c,"FLUSHALL");
	printf("FLUSHALL reply %s\n", reply->str);
    freeReplyObject(reply);
}
int handle_request(redisContext *c,const char*method,const char*url,const char*body,int validate,char*result)
{
	jsonEntry je;
	request r;
	int ret;
    ret=parse_method(method,method+strlen(method));
    if(!ret)
		return -1;
	switch(ret)
	{
	    case GET_METHOD   :
		{
			printf("GET method\n");
			ret=parse_get_url(url,url+strlen(url),&r);
			if(!ret)
				return -2;



			switch(ret)
			{
				case GET_ALL:
					printf("GET ALL\n");
					get_all(c,result);
					break;
				case GET_BY_VALUE:
					printf("GET BY VALUE\n");
					get_by_value(c,result);
					break;
				case GET_BY_ID:{
			        char id[128];
			        memcpy( id, url, r.id.start );
			        id[r.id.end+1] = '\0';
					printf("GET BY ID\n");
					get_by_id(c,result, id);
					break;
				}
				case GET_ITEMS:
					printf("GET ITEMS\n");
					get_items(c);
					break;
				case GET_TRANSACTIONS:
					printf("GET TRANSACTIONS\n");
					get_transactions(c);
					break;
			}
			break;
		}
	    case POST_METHOD  : 
		{
			printf("POST method\n");
			
			ret=analise_json(c,&je,body,validate);
		    if(ret!=JSMN_SUCCESS)
			{
				clear_json(&je);
		        return -3;
			}
			//handle_post
			post(c,&je);
			clear_json(&je);
			break;
		}
	    case PUT_METHOD   : 
		{
			printf("PUT method\n");
			ret=parse_put_url(url,url+strlen(url),&r);
			if(!ret)
				return -2;
			switch(ret)
			{
				case PUT_ALL:
					printf("PUT ALL\n");
					break;
				case PUT_BY_ID:
					printf("PUT BY ID\n");
					break;
				case PUT_TRANSACTIONS:
					printf("PUT TRANSACTIONS\n");
					break;
			}
			ret=analise_json(c,&je,body,validate);
		    if(ret!=JSMN_SUCCESS)
			{
				clear_json(&je);
		        return -3;
			}
			//handle put
			clear_json(&je);
			break;
		}
	    case DELETE_METHOD: 
		{
			printf("DELETE method\n");
			//handle_delete
			delete_all(c);
			break;
		}
	}
	return 0;	
}
