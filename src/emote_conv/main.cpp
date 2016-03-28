#include "def.h"
#include "lz4.h"
#include "lz4frame.h"

#define LZ4_MAGIC 0x184D2204

#define LZ4_HEADER_SIZE 19
#define LZ4_FOOTER_SIZE 4

string source_file;

struct EMoteCTX
{
	uint32_t key[4];
	uint32_t v;
	uint32_t count;
};

void init_emote_ctx(EMoteCTX* ctx,uint32_t key[4])
{
	ctx->key[0] = key[0];
	ctx->key[1] = key[1];
	ctx->key[2] = key[2];
	ctx->key[3] = key[3];
	ctx->v = 0;
	ctx->count = 0;
}


void emote_decrypt(EMoteCTX* ctx,unsigned char *data, uint32_t length)
{

	if(length > 0)
	{
		for(uint32_t i=0;i<length;i++)
		{
			if(!ctx->v){
				uint32_t a,b,c;
				b = ctx->key[3];
				a = ctx->key[0] ^ (ctx->key[0] << 0xB);
				ctx->key[0] = ctx->key[1];
				ctx->key[1] = ctx->key[2];
				c = a ^ b ^ ((a ^ (b >> 0xB)) >> 8);
				ctx->key[2] = b;
				ctx->key[3] = c;
				ctx->v = c;
				ctx->count = 4;
			}

			data[i] ^= (uint8_t)ctx->v;
			ctx->v >>= 8;
			ctx->count--;
		}
	}
}


void decrypt_encrypt_psb(unsigned char *data, uint32_t length)
{
	PSBHDR *hdr = (PSBHDR*)data;
	EMoteCTX emoteCtx;
	uint32_t key[4];
	if(memcmp(hdr->signature,"PSB",3) != 0){
		cout << "invalid file signature" << endl;
		exit(1);
	}
	uint32_t hdr_size = sizeof(PSBHDR);

	if(hdr->version == 3) {
		hdr_size = sizeof(PSBHDR3);
	}

	unsigned char *dec_hdr = new unsigned char[hdr_size];
	memcpy(dec_hdr,hdr,hdr_size);

	key[0] = 0x075BCD15;
	key[1] = 0x159A55E5;
	key[2] = 0x1F123BB5;
	key[3] = 0x174E897D;

	init_emote_ctx(&emoteCtx,key);

	if(hdr->encrypt){
		uint32_t start_of = (char*)&hdr->offset_encrypt - (char*)&hdr->signature;
		emote_decrypt(&emoteCtx,&dec_hdr[start_of],hdr_size - start_of);
		memcpy(data,dec_hdr,hdr_size);
	}

	if(hdr->version == 2)
	{
		emote_decrypt(&emoteCtx,&data[hdr->offset_encrypt],hdr->offset_chunk_offsets - hdr->offset_encrypt);
	}
}

void process_decompress(unsigned char *data, uint32_t length)
{
	LZ4F_decompressionContext_t context;
	LZ4F_frameInfo_t frameInfo;
	LZ4F_errorCode_t err;
	size_t srcSize = length;
	unsigned char *uncompr_data;
	uint32_t pos,dstPos;
	uint32_t dstSize;

	cout << "decompress..." << endl;
	err = LZ4F_createDecompressionContext(&context,LZ4F_VERSION);
	if(LZ4F_isError(err)){
		cout << "LZ4F_createDecompressionContext failed:" << LZ4F_getErrorName(err) << endl;
		exit(1);
	}

	err = LZ4F_getFrameInfo(context,&frameInfo,data,&srcSize);
	if(LZ4F_isError(err)){
		cout << "LZ4F_getFrameInfo failed:" << LZ4F_getErrorName(err) << endl;
		exit(1);
	}

	pos = srcSize;
	dstPos = 0;
	uncompr_data = new unsigned char[(uint32_t)frameInfo.contentSize];

	do 
	{
		dstSize = (uint32_t)frameInfo.contentSize - dstPos;
		srcSize = length - pos;

		err = LZ4F_decompress(context,&uncompr_data[dstPos],&dstSize,&data[pos],&srcSize,NULL);

		if(LZ4F_isError(err)){
			cout << "LZ4F_decompress failed:" << LZ4F_getErrorName(err) << endl;
			exit(1);
		}

		dstPos += dstSize;
		pos += srcSize;
	} while (err);
	cout << "decrypt_encrypt_psb" << endl;
	decrypt_encrypt_psb(uncompr_data,(uint32_t)frameInfo.contentSize);
	fstream output(source_file,ios::out|ios::binary|ios::trunc);
	output.write((char*)uncompr_data,(uint32_t)frameInfo.contentSize);
	output.flush();output.close();
	cout << "decode ok" << endl;
}

void process_compress(unsigned char *data, uint32_t length)
{
	LZ4F_compressionContext_t context;
	LZ4F_errorCode_t err;
	
	size_t dstSize;
	unsigned char *dstBuf;
	uint32_t pos = 0;

	static const LZ4F_preferences_t lz4_preferences = {
		{ LZ4F_max4MB, LZ4F_blockLinked, LZ4F_noContentChecksum, LZ4F_frame, length, { 0, 0 } },
		0,   /* compression level */
		0,   /* autoflush */
		{ 0, 0, 0, 0 },  /* reserved, must be set to 0 */
	};

	cout << "encode psb..." << endl;
	decrypt_encrypt_psb(data,(uint32_t)length);

	cout << "compress..." << endl;
	err = LZ4F_createCompressionContext(&context,LZ4F_VERSION);

	if(LZ4F_isError(err)){
		cout << "LZ4F_createCompressionContext failed:" << LZ4F_getErrorName(err) << endl;
		exit(1);
	}

	dstSize = LZ4F_compressBound(length,&lz4_preferences) + LZ4_HEADER_SIZE + LZ4_FOOTER_SIZE;
	dstBuf = new unsigned char[dstSize];

	err = LZ4F_compressBegin(context,dstBuf,dstSize,&lz4_preferences);
	if (LZ4F_isError(err)) {
		cout << "Failed to start compression:"<< LZ4F_getErrorName(err) << endl;
		exit(1);
	}

	pos = err;

	
	err = LZ4F_compressUpdate(context, dstBuf+pos, dstSize -pos, data, length, NULL);

	if (LZ4F_isError(err)) {
		cout << "Failed to LZ4F_compressUpdate:"<< LZ4F_getErrorName(err) << endl;
		exit(1);
	}

	pos += err;

	err = LZ4F_compressEnd(context,dstBuf+pos,dstSize-pos,NULL);

	if (LZ4F_isError(err)) {
		cout << "Failed to LZ4F_compressEnd:"<< LZ4F_getErrorName(err) << endl;
		exit(1);
	}

	pos += err;

	fstream output(source_file,ios::binary|ios::out|ios::trunc);
	if(output.is_open())
	{
		output.write((char*)dstBuf,pos);
		output.flush();
		output.close();

		cout << "ok" << endl;
	}
	else{
		cout << "can't open output file" << endl;
	}
}

int main(int argc, char* argv[])
{
	uint32_t length;
	if(argc < 2){
		cout << "emote_conv <filename.psb>" << endl;
		return 0;
	}

	source_file = argv[1];
	fstream file(source_file,ios::binary|ios::in);
	if(!file.is_open()){
		cout << "open file failed" << endl;
		return 1;
	}

	file.seekg(0,ios::end);
	length = (uint32_t)file.tellg();
	file.seekg(0,ios::beg);
	unsigned char *data =new unsigned char[length];
	file.read((char*)data,length);
	file.close();

	if(*(uint32_t*)data == LZ4_MAGIC){
		process_decompress(data,length);
	}
	else{
		process_compress(data,length);
	}
	return 0;
}