// Netstats.c  - Network traffic statistics
//

#include "U-USER.H"

#if DEBUG

typedef struct {
	unsigned long eventType;
	unsigned long nbrEvents;
	unsigned long lengthEvents;
} NetStatEntry;

NetStatEntry	gNetStats[100];
int				gNbrStatEvents;

void InitNetStats();
void ReportNetStats();
void TallyNetStat(unsigned long eventType, long eventLength);

void ReportNetStats()
{
	int				i;
	NetStatEntry	*ns;
	ns = gNetStats;
	for (i = 0; i < gNbrStatEvents; ++i,++ns) {
		LogMessage("%.4s %6ld %9ld\r",&ns->eventType,(long) ns->nbrEvents,(long) ns->lengthEvents);
	}
}

void TallyNetStat(unsigned long eventType, long eventLength)
{
	int				i;
	Boolean			gotOne = false;
	NetStatEntry	*ns,temp;

	ns = gNetStats;
	for (i = 0; i < gNbrStatEvents; ++i,++ns) {
		if (ns->eventType == eventType) {
			gotOne = true;
			break;
		}
	}
	if (!gotOne && gNbrStatEvents < 100) {
		ns->eventType = eventType;
		ns->nbrEvents = 0;
		ns->lengthEvents = 0;
		++gNbrStatEvents;
	}
	ns->nbrEvents++;
	ns->lengthEvents += eventLength+12;
	// Swap in to first position
	temp = gNetStats[0];
	gNetStats[0] = *ns;
	*ns = temp;
}

#endif