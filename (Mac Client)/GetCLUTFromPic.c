/* Get a Palette from a PicHandle */
CTabHandle GetCLUTFromPic(PicHandle	pic);

CTabHandle GetCLUTFromPic(PicHandle	pic)
{
	Ptr				p;
	register short	opCode;
	Handle			ch;

	if (pic == NULL || *pic == NULL)
		return NULL;

	HLock((Handle) pic);
	p = (Ptr) *pic;

	p += sizeof(short);		// Skip Size
	p += sizeof(Rect);		// Skip Frame
	
	while (1) {
		opCode = *((short *) p); p += sizeof(short);
		if (opCode >= 0x8100) {
			long	t;
			t = *((long *) p);	p += sizeof(long);
			p += t;
		}
		else {
			switch (opCode) {
			case 0x0004:
				p += 1;
				break;
			case 0x0003:
			case 0x0005:
			case 0x0008:
			case 0x0011:
			case 0x000D:
			case 0x0015:
			case 0x0016:
			case 0x0023:
			case 0x00A0:
			case 0x0100:
			case 0x01FF:
				p += 2;	
				break;
			case 0x0006:
			case 0x0007:
			case 0x000B:
			case 0x000C:
			case 0x000E:
			case 0x000F:
			case 0x0021:
			case 0x0068:
			case 0x0069:
			case 0x006A:
			case 0x006B:
			case 0x006C:
			case 0x006D:
			case 0x006E:
			case 0x006F:
			case 0x0200:
				p += 4;
				break;
			case 0x0002:
			case 0x0009:
			case 0x000A:
			case 0x0010:
			case 0x0020:
			case 0x0030:
			case 0x0031:
			case 0x0032:
				p += 8;
				break;
			case 0x0012:
			case 0x0013:
			case 0x0014:
				p += 16;
				break;
			case 0x0C00:	// header
				p += 24;
				break;
			case 0x0001:
			case 0x0080:
			case 0x0081:
			case 0x0082:
			case 0x0083:
			case 0x0084:	// Regions
				p += *((short *) p);
				break;

			case 0x009A:
			case 0x009B:
			case 0x009C:
			case 0x009D:
			case 0x009E:
			case 0x009F:
			case 0x00A2:
				{
					short	t;
					t = *((short *) p);	p += sizeof(short);
					p += t;
				}
				break;
			case 0x00A1:
			case 0xFFFF:
				{
					long	t;
					t = *((long *) p);	p += sizeof(long);
					p += t;
				}
				break;
			case 0x0090:
			case 0x0091:
			case 0x0098:
			case 0x0099:
				{
					p += sizeof(PixMap)-4;	// PixMap
					goto Success;
				}
				break;
			case 0x00FF:	// End of Pict
				goto Failure;
			}
		}
	}
Failure:
	HUnlock((Handle) pic);
	return NULL;

Success:
	ch = NewHandle(sizeof(ColorTable) + sizeof(ColorSpec)*255);
	if (ch == NULL)
		return NULL;
	BlockMove(p,*ch,sizeof(ColorTable));
	BlockMove(p+8,*ch+8,(*((short *) (p+6)) + 1)*sizeof(ColorSpec));
	HUnlock((Handle) pic);
	return (CTabHandle) ch;
}