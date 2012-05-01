#ifdef _MSC_VER
# define STRUCT_PACKED
# pragma pack(push, 1)
#else
# define STRUCT_PACKED __attribute__((packed))
#endif

struct header
{
    int magic;              // 0x00  4 bajty  string "RC03"
    int res1;               // 0x04  4 bajty  ??? - u mnie wynosi 1
    int index_off;          // 0x08  4 bajty  offset "indeksu" w pliku
    int index_size;         // 0x0C  4 bajty  rozmiar "indeksu" (w bajtach)
    int size;               // 0x10  4 bajty  rozmiar całego pliku (po co?)
    int data_off;           // 0x14  4 bajty  offset "sekcji danych" w pliku
    int res2[3];            // 0x18  12 bajtów  ??? - u mnie 00 10 00 00 00 10 00 00 00 00 00 00
    int uin;                // 0x24  4 bajty  UIN właściciela XOR 0xFFFFFD66
    int control_sum;        // 0x28  4 bajty  suma kontrolna
} STRUCT_PACKED;


struct index
{
    int section;            // 0x00  4 bajty  numer sekcji
    int blocks;             // 0x04  4 bajty  ilość "bloków" tworzących sekcję
    int first_block_off;    // 0x08  4 bajty  offset do pierwszego bloku
    int last_block_off;     // 0x0C  4 bajty  offset do ostatniego bloku
    int free_space;         // 0x10  4 bajty  ilość wolnego miejsca (w jakich jednostkach?)
} STRUCT_PACKED;


/* offsety względem header.data_off */
struct block
{
    int control_sum;        // 0x00  4 bajty  suma kontrolna
    int section;            // 0x04  4 bajty  numer sekcji
    int len;                // 0x08  4 bajty  długość bloku (w bajtach)
    int next_block;         // 0x0C  4 bajty  offset do następnego bloku
    int block_size;         // 0x10  4 bajty  ilość "danych" w bloku (w bajtach)
} STRUCT_PACKED;

struct msg_header
{
    int flags;              // 0x00  4 bajty  flagi?
    int msg_off;            // 0x04  4 bajty  offset do samej wiadomości
    int msg_size;           // 0x08  4 bajty  rozmiar wiadomości
    int block_off;          // 0x0C  4 bajty  offset do początku bloku
} STRUCT_PACKED;


struct message
{
    int send_time;           // 0x00  4 bajty  czas wysłania wiadomości
    unsigned int sender_uin; // 0x04  4 bajty  numer nadawcy
    int recivers;            // 0x08  4 bajty  ilość odbiorców (n)
} STRUCT_PACKED;

struct rcv_msg              //wiadomość przychodząca
{
    int rcv_time;           // 0x0C  4 bajty  czas odebrania wiadomości
    int len;                // 0x10  4 bajty  długość wiadomości (len)
} STRUCT_PACKED;

#ifdef _MSC_VER
# pragma pack(pop)
#endif
