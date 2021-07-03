#ifndef I2C_H_
#define I2C_H_

#define TWI_BUFFER_MAX					35 
#define TWI_USEFULL_BUFFER_MAX	(TWI_BUFFER_MAX - 3) 
#define TWI_SEGMENTS_MAX				7
#define TWI_TABLE_SIZE					(sizeof(twitab_t) + TWI_SEGMENTS_MAX * sizeof(twi_req_t) + 1)

#define TWI_REQ_WRITE						0
#define TWI_REQ_WRITE_BLOCK			1
#define TWI_REQ_READ						2
#define TWI_REQ_READ_BLOCK			3

#ifdef NPSYSTEM_FIRMWARE_API_CPP
#	pragma warning(disable:4200)  
#	pragma pack(push, 1)
#endif // NPSYSTEM_FIRMWARE_API_CPP

typedef struct __twi_req_t {
	uint8_t fn;
	uint8_t dev_addr;
	union
	{
		uint16_t addr;
		struct {
			uint8_t _addrl;
			uint8_t _addrh;
		};
	};
	union
	{
#ifdef NPSYSTEM_FIRMWARE_API_CPP
		uint16_t data;
#else
		uint8_t* data;
#endif // NPSYSTEM_FIRMWARE_API_CPP
		uint8_t wr_value;
	};
	uint8_t len;
	uint8_t status;

#ifdef NPSYSTEM_FIRMWARE_API_CPP
	template<class Archive>
	void serialize(Archive & ar, const unsigned int file_version)
	{
		ar & fn;
		ar & dev_addr;
		ar & addr;
		ar & data;
		ar & len;
		ar & status;
	}
#endif // NPSYSTEM_FIRMWARE_API_CPP

} twi_req_t;

typedef void(*twi_fun)(twi_req_t*);

#ifndef NPSYSTEM_FIRMWARE_API_CPP
typedef struct {
	uint8_t n;
	twi_req_t twi_req[];
} twitab_t;
#else 
struct twitab_t {
	uint8_t n;
	twi_req_t twi_req[];
};
#endif // NPSYSTEM_FIRMWARE_API_CPP

#ifdef NPSYSTEM_FIRMWARE_API_CPP
#pragma warning(default:4200)  
#pragma pack(pop)
#endif // NPSYSTEM_FIRMWARE_API_CPP

#ifndef NPSYSTEM_FIRMWARE_API_CPP
//////////////////////////////////////////////////////////////////////////
#define TWI_FREE					0
#define TWI_SUCCESS				((int8_t)1)
#define TWI_BUSY					((int8_t)2)
#define TWI_ERROR					((int8_t)-1)
//////////////////////////////////////////////////////////////////////////
void twi_init(void);
void twi_write(twi_req_t* req);
void twi_read(twi_req_t* req);
void twi_write_bytes(twi_req_t* req);
void twi_read_bytes(twi_req_t* req);

extern volatile uint8_t twi_result;
extern uint8_t twi_buf[TWI_BUFFER_MAX];
#endif // NPSYSTEM_FIRMWARE_API_CPP

#endif // I2C_H_ 