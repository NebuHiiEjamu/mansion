type 'PUZZ' {
		integer = $$Countof(StringArray);
		array StringArray {
				pstring;										/* String				*/
		};
};
type 'STR#' {
		integer = $$Countof(StringArray);
		array StringArray {
				pstring;										/* String				*/
		};
};

resource 'STR#' (129, "GrandPrizes", purgeable) {
{
"100000,a fully loaded PowerPC computer, courtesy of Apple Computer + cash",
"100000,a Silicon Graphics Indigo with Extreme graphics, courtesy of SGI + cash",
"100000,a used Connection Machine I with 128 processors",
"100000,a home entertainment center from Sears + cash",
"100000,a slightly used Mazda Miata + cash",
"100000,a brand new Corvette + cash",
"100000,a brand new Cadillac Eldorado Touring Coupe + cash",
"100000,a brand new BMW 850CSi",
"100000,a used Bugatti EB 110 sports car",
"100000, a brand new Dodge Viper RT/10 + cash",
"100000,a brand new Acura Integra + cash",
"100000,a brand new Ford Explorer + cash",
"100000,a cruise for 2 to Tahiti, courtesy of Princess Cruises + cash",
"100000,a trip to beautiful Burbank, California + cash",
"100000,a date with Vanna's sister, Pat",
"100000,one of Vanna's chromosomes, encased in plastic",
"100000,a designer pink coat",
"100000,a steinway grand piano + cash",
"100000,music studio equipment by roland",
"100000,a beautiful master bedroom suite + cash",
"100000,a date with me, Patbot"
}
};

resource 'STR#' (128, "SmallPrizes", purgeable) {
{
"20,a set of faux-crystal wine glasses",
"4,a big jumbo cheese log",
"1400,a pink grandfather clock, courtesy of swatch",
"2,a pair of slightly used jockey shorts",
"44,an extremely shiny, polished, piece of shit",
"1,an extremely common Tanzanian postage stamp of Elvis",
"400,hand carved velour blinds",
"200,a medium sized color TV by Sears",
"800,a large screen color TV by Sony",
"15,an astroturf welcome mat with plastic daisies",
"99,a Walt Disney action figure, encased in a lucite slab",
"45,a small black and white TV, made by GE",
"10,a can of Franklin Spelling Mace",
"25,a Timex Sinclair 1000 computer",
"500,a cheesy IBM PC clone",
"50,an inflatable plastic doll",
"75,a casio electronic organ",
"69,a set of Jesse Helms action figures",
"100,a set of tacky porcelain figurines, courtesy of the Franklin Mint",
"99,an Elvis dinner plate, courtesy of the Franklin Mint",
"20,a beautiful wrist watch, courtesy of Timex",
"400,a complete Funk & Wagnall's reference set"
}};

resource 'STR#' (200, "Auto Ops", purgeable) {
{
"bennyboy","no-one","muffin","praetor","stumbo","alison",
"batman","tara","isis"}};

resource 'STR#' (201,"Auto Nix", purgeable) {
{"lavender>lav","icekool>ice","bennyboy>bb","roybatty>roy","aladdin>al"
}};

resource 'PUZZ' (128, "Movie", purgeable) {
	{	/* array StringArray: 203 elements */
		/* [1] */
		"A CLOCKWORK ORANGE",
		/* [2] */
		"A NIGHTMARE ON ELM STREET",
		/* [3] */
		"AIRPLANE!",
		/* [4] */
		"AMAZON WOMEN ON THE MOON",
		/* [5] */
		"AMERICAN GRAFFITI",
		/* [6] */
		"AMERICAN WEREWOLF IN LONDON",
		/* [7] */
		"AN AMERICAN IN PARIS",
		/* [8] */
		"AND THEN THERE WERE NONE",
		/* [9] */
		"ANGEL IN MY POCKET",
		/* [10] */
		"ANIMAL HOUSE",
		/* [11] */
		"ARTHUR",
		/* [12] */
		"ATTACK OF THE KILLER TOMATOES",
		/* [13] */
		"BACHELOR PARTY",
		/* [14] */
		"BATMAN",
		/* [15] */
		"BEACH BLANKET BINGO",
		/* [16] */
		"BEVERLY HILLS COP",
		/* [17] */
		"BIG",
		/* [18] */
		"BIG TOP PEE-WEE",
		/* [19] */
		"BLADE RUNNER",
		/* [20] */
		"BLUES BROTHERS",
		/* [21] */
		"BODY DOUBLE",
		/* [22] */
		"BOLERO",
		/* [23] */
		"BONNIE AND CLYDE",
		/* [24] */
		"BRAINSTORM",
		/* [25] */
		"CADDYSHACK",
		/* [26] */
		"CANNON BALL RUN",
		/* [27] */
		"CASABLANCA",
		/* [28] */
		"CITIZEN KANE",
		/* [29] */
		"CITY HEAT",
		/* [30] */
		"CLASS REUNION",
		/* [31] */
		"CLUE",
		/* [32] */
		"CRAZY PEOPLE",
		/* [33] */
		"CREEPSHOW",
		/* [34] */
		"DARKMAN",
		/* [35] */
		"DEAD MEN DON'T WEAR PLAID",
		/* [36] */
		"DEAD POETS SOCIETY",
		/* [37] */
		"DESK SET",
		/* [38] */
		"DESPERATELY SEEKING SUSAN",
		/* [39] */
		"DIRTY HARRY",
		/* [40] */
		"DR. DETROIT",
		/* [41] */
		"DR. JECKYL AND MR. HYDE",
		/* [42] */
		"DR. NO",
		/* [43] */
		"E.T. THE EXTRA-TERRESTRIAL",
		/* [44] */
		"ELECTRIC DREAMS",
		/* [45] */
		"ELVIRA - MISTRESS OF THE DARK",
		/* [46] */
		"END OF THE LINE",
		/* [47] */
		"ERASERHEAD",
		/* [48] */
		"ESCAPE FROM NEW YORK",
		/* [49] */
		"FAST TIMES AT RIDGEMONT HIGH",
		/* [50] */
		"FISH HEADS",
		/* [51] */
		"FIST FULL OF DOLLARS",
		/* [52] */
		"FROM HERE TO ETERNITY",
		/* [53] */
		"FROM RUSSIA WITH LOVE",
		/* [54] */
		"FROSTY THE SNOWMAN",
		/* [55] */
		"GARGOYLES",
		/* [56] */
		"GHIDRA THE THREE-HEADED MONSTER",
		/* [57] */
		"GHOST BUSTERS",
		/* [58] */
		"GODZILLA RAIDS AGAIN",
		/* [59] */
		"GODZILLA VS. GIGAN",
		/* [60] */
		"GODZILLA VS. MOTHRA",
		/* [61] */
		"GODZILLA VS. THE SEA MONSTER",
		/* [62] */
		"GODZILLA VS. THE SMOG MONSTER",
		/* [63] */
		"GODZILLA'S REVENGE",
		/* [64] */
		"GODZILLA, KING OF THE MONSTERS",
		/* [65] */
		"GOING BERSERK",
		/* [66] */
		"GREASE",
		/* [67] */
		"GUNG HO",
		/* [68] */
		"HEAVEN CAN WAIT",
		/* [69] */
		"HOT DOG",
		/* [70] */
		"INVASION OF THE BODY SNATCHERS",
		/* [71] */
		"IT'S A WONDERFUL LIFE",
		/* [72] */
		"IT'S MAD, MAD, MAD, MAD WORLD",
		/* [73] */
		"JAWS",
		/* [74] */
		"JOHNNY GOT HIS GUN",
		/* [75] */
		"KENTUCKY FRIED MOVIE",
		/* [76] */
		"KILLER KLOWNS FROM OUTER SPACE",
		/* [77] */
		"KING KONG VS. GODZILLA",
		/* [78] */
		"LASSITER",
		/* [79] */
		"LIQUID SKY",
		/* [80] */
		"LIVE AND LET DIE",
		/* [81] */
		"MAD MAX",
		/* [82] */
		"MASH",
		/* [83] */
		"MAX HEADROOM, TWENTY MINUTES INTO THE FU"
		"TURE",
		/* [84] */
		"MEATBALLS",
		/* [85] */
		"MEMPHIS BELLE",
		/* [86] */
		"MIRACLE ON THIRTY-FOURTH STREET",
		/* [87] */
		"MR. HOBBES TAKES A VACATION",
		/* [88] */
		"MR. MOM",
		/* [89] */
		"MUNSTER GO HOME",
		/* [90] */
		"MURDER BY DEATH",
		/* [91] */
		"MY FAVORITE YEAR",
		/* [92] */
		"MY TUTOR",
		/* [93] */
		"NEIGHBORS",
		/* [94] */
		"NIGHT SHIFT",
		/* [95] */
		"NIGHTMARE ON ELM STREET",
		/* [96] */
		"NO TIME FOR SERGEANTS",
		/* [97] */
		"NORTH BY NORTHWEST",
		/* [98] */
		"ON THE WATERFRONT",
		/* [99] */
		"OPERATION PETTICOAT",
		/* [100] */
		"PAINT YOUR WAGON",
		/* [101] */
		"PEE-WEE'S BIG ADVENTURE",
		/* [102] */
		"PENNIES FROM HEAVEN",
		/* [103] */
		"POLICE ACADEMY",
		/* [104] */
		"PREDATOR",
		/* [105] */
		"PSYCHO",
		/* [106] */
		"RADAR MEN FROM THE MOON",
		/* [107] */
		"RAIDERS OF THE LOST ARK",
		/* [108] */
		"RAISING ARIZONA",
		/* [109] */
		"RAMBO",
		/* [110] */
		"REAR WINDOW",
		/* [111] */
		"REBEL WITHOUT A CAUSE",
		/* [112] */
		"RECKLESS",
		/* [113] */
		"REPO MAN",
		/* [114] */
		"RETURN OF THE JEDI",
		/* [115] */
		"RETURN TO MAYBERRY",
		/* [116] */
		"REVENGE OF THE NERDS",
		/* [117] */
		"RISKY BUSINESS",
		/* [118] */
		"ROAD WARRIOR",
		/* [119] */
		"ROCK AND ROLL HIGH SCHOOL",
		/* [120] */
		"RUMBLE FISH",
		/* [121] */
		"SANDS OF IWO JIMA",
		/* [122] */
		"SEVEN YEAR ITCH",
		/* [123] */
		"SHAMPOO",
		/* [124] */
		"SHARKY'S MACHINE",
		/* [125] */
		"SHERLOCK HOLMES AND THE SECRET WEAPON",
		/* [126] */
		"SILENT MOVIE",
		/* [127] */
		"SILVER STREAK",
		/* [128] */
		"SIXTEEN CANDLES",
		/* [129] */
		"SOME LIKE IT HOT",
		/* [130] */
		"SON OF GODZILLA",
		/* [131] */
		"SPACE BALLS",
		/* [132] */
		"SPLASH",
		/* [133] */
		"STAR WARS",
		/* [134] */
		"STATE FAIR",
		/* [135] */
		"STIR CRAZY",
		/* [136] */
		"STRANGE BREW",
		/* [137] */
		"STRIPES",
		/* [138] */
		"STROKER ACE",
		/* [139] */
		"SUDDEN IMPACT",
		/* [140] */
		"SUMMER LOVERS",
		/* [141] */
		"TEN",
		/* [142] */
		"TERMINATOR II",
		/* [143] */
		"THE ABYSS",
		/* [144] */
		"THE ADVENTURES OF BUCKAROO BANZAI",
		/* [145] */
		"THE AFRICAN QUEEN",
		/* [146] */
		"THE APARTMENT",
		/* [147] */
		"THE BIRDMAN OF ALCATRAZ",
		/* [148] */
		"THE BLUE MAX",
		/* [149] */
		"THE COMPUTER WORE TENNIS SHOES",
		/* [150] */
		"THE DAY THE EARTH STOOD STILL",
		/* [151] */
		"THE DIRTY DOZEN",
		/* [152] */
		"THE EMPIRE STRIKES BACK",
		/* [153] */
		"THE FLY",
		/* [154] */
		"THE GEORGE WHITE FOLLIES",
		/* [155] */
		"THE GHOST AND MR. CHICKEN",
		/* [156] */
		"THE GLENN MILLER STORY",
		/* [157] */
		"THE GOOD, THE BAD, THE UGLY",
		/* [158] */
		"THE GREAT TRAIN ROBBERY",
		/* [159] */
		"THE HUNGER",
		/* [160] */
		"THE JERK",
		/* [161] */
		"THE LAST AMERICAN VIRGIN",
		/* [162] */
		"THE LAST STARFIGHTER",
		/* [163] */
		"THE LONELY GUY",
		/* [164] */
		"THE LONGEST DAY",
		/* [165] */
		"THE MAN IN THE GREY FLANNEL SUIT",
		/* [166] */
		"THE MAN WITH THE GOLDEN GUN",
		/* [167] */
		"THE MAN WITH TWO BRAINS",
		/* [168] */
		"THE NUTTY PROFESSOR",
		/* [169] */
		"THE OLD MAN AND THE SEA",
		/* [170] */
		"THE POSTMAN ALWAYS RINGS TWICE",
		/* [171] */
		"THE PRIVATE EYES",
		/* [172] */
		"THE RHYTHMATIST",
		/* [173] */
		"THE ROCKETEER",
		/* [174] */
		"THE SHINING",
		/* [175] */
		"THE STING",
		/* [176] */
		"THE TERMINATOR",
		/* [177] */
		"THE WIZARD OF OZ",
		/* [178] */
		"THE WORLD ACCORDING TO GARP",
		/* [179] */
		"THIS ISLAND EARTH",
		/* [180] */
		"THUNDERHEART",
		/* [181] */
		"TO LIVE AND DIE IN L.A.",
		/* [182] */
		"TO RUSSIA WITH LOVE",
		/* [183] */
		"TOOLMASTER OF BRANARD",
		/* [184] */
		"TOP SECRET",
		/* [185] */
		"TRINITY IS STILL MY NAME",
		/* [186] */
		"TUCKER - THE MAN AND HIS DREAM",
		/* [187] */
		"TURNER AND HOOCH",
		/* [188] */
		"TWILIGHT ZONE",
		/* [189] */
		"TWO-THOUSAND ONE, A SPACE ODYSSEY",
		/* [190] */
		"UNFAITHFULLY YOURS",
		/* [191] */
		"VACATION",
		/* [192] */
		"VALLEY GIRL",
		/* [193] */
		"VAMP",
		/* [194] */
		"VIDEODROME",
		/* [195] */
		"WAR GAMES",
		/* [196] */
		"WAR OF THE WORLDS",
		/* [197] */
		"WEIRD SCIENCE",
		/* [198] */
		"WHITE CHRISTMAS",
		/* [199] */
		"WHO FRAMED ROGER RABBIT?",
		/* [200] */
		"WITHOUT A CLUE",
		/* [201] */
		"YOUNG DOCTORS IN LOVE",
		/* [202] */
		"YOUNG FRANKENSTEIN",
		/* [203] */
		"ZAPPED"
	}
};

resource 'PUZZ' (129, "Actor or Actress", purgeable) {
	{	/* array StringArray: 350 elements */
		/* [1] */
		"ADRIENNE BARBEAU",
		/* [2] */
		"AIDAN QUINN",
		/* [3] */
		"AL LEWIS",
		/* [4] */
		"ALAN ALDA",
		/* [5] */
		"ALAN KING",
		/* [6] */
		"ALEC GUINNESS",
		/* [7] */
		"ALLEN HALE",
		/* [8] */
		"ALLY SHEEDY",
		/* [9] */
		"AMANDA PAYS",
		/* [10] */
		"ANDY GRIFFITH",
		/* [11] */
		"ANETA CORSAUT",
		/* [12] */
		"ANNIE POTTS",
		/* [13] */
		"ANTHONY MICHAEL HALL",
		/* [14] */
		"ANTHONY PERKINS",
		/* [15] */
		"ARETHA FRANKLIN",
		/* [16] */
		"ARNOLD SCHWARTZENEGAR",
		/* [17] */
		"BARBARA BILLINGSLY",
		/* [18] */
		"BARNES AND BARNES",
		/* [19] */
		"BASIL RATHBONE",
		/* [20] */
		"BEN KINGSLEY",
		/* [21] */
		"BERNADETTE PETERS",
		/* [22] */
		"BERNIE CASEY",
		/* [23] */
		"BERT CONVY",
		/* [24] */
		"BERT LAHR",
		/* [25] */
		"BETTY LYNN",
		/* [26] */
		"BEVERLY D'ANGELO",
		/* [27] */
		"BIANCA JAGGER",
		/* [28] */
		"BILL MURRAY",
		/* [29] */
		"BILLY DEE WILLIAMS",
		/* [30] */
		"BING CROSBY",
		/* [31] */
		"BO DEREK",
		/* [32] */
		"BO HOPKINS",
		/* [33] */
		"BOB GELDOF",
		/* [34] */
		"BOB HASTINGS",
		/* [35] */
		"BOB HOSKINS",
		/* [36] */
		"BONES MALONE",
		/* [37] */
		"BRIAN KEITH",
		/* [38] */
		"BUBBA SMITH",
		/* [39] */
		"BUCK HENRY",
		/* [40] */
		"BUDDY FOSTER",
		/* [41] */
		"BURT REYNOLDS",
		/* [42] */
		"BUTCH PATICK",
		/* [43] */
		"CAB CALOWAY",
		/* [44] */
		"CAROLYN JONES",
		/* [45] */
		"CARY GRANT",
		/* [46] */
		"CATHERINE DENEUVE",
		/* [47] */
		"CHARLES GRODIN",
		/* [48] */
		"CHEVY CHASE",
		/* [49] */
		"CHRISTIE BRINKLEY",
		/* [50] */
		"CHRISTOPHER LLOYD",
		/* [51] */
		"CHRISTOPHER PENN",
		/* [52] */
		"CINDY WILLIAMS",
		/* [53] */
		"CLAUDE RAINS",
		/* [54] */
		"CLINT EASTWOOD",
		/* [55] */
		"CLINT HOWARD",
		/* [56] */
		"CLORIS LEACHMAN",
		/* [57] */
		"CONNIE STEVENS",
		/* [58] */
		"CORNEL WILDE",
		/* [59] */
		"CRISPIN GLOVER",
		/* [60] */
		"DABNEY COLEMAN",
		/* [61] */
		"DAN AYKROYD",
		/* [62] */
		"DANNY KAYE",
		/* [63] */
		"DAVE THOMAS",
		/* [64] */
		"DAVID BOWIE",
		/* [65] */
		"DAVID LETTERMAN",
		/* [66] */
		"DAVID NIVEN",
		/* [67] */
		"DEAN MARTIN",
		/* [68] */
		"DEBBIE WATSON",
		/* [69] */
		"DEBORAH FOREMAN",
		/* [70] */
		"DEE WALLACE",
		/* [71] */
		"DEFOREST KELLEY",
		/* [72] */
		"DENNIS HOPPER",
		/* [73] */
		"DENVER PYLE",
		/* [74] */
		"DICK SARGENT",
		/* [75] */
		"DOM DELOUIS",
		/* [76] */
		"DOMINO",
		/* [77] */
		"DON KNOTTS",
		/* [78] */
		"DONALD SUTHERLAND",
		/* [79] */
		"DONNA DIXON",
		/* [80] */
		"DONNA REED",
		/* [81] */
		"DOUG DILLARD",
		/* [82] */
		"DREW BARRYMORE",
		/* [83] */
		"DUB TAYLOR",
		/* [84] */
		"DUCK DUNN",
		/* [85] */
		"DUDLEY MOORE",
		/* [86] */
		"DUSTIN HOFFMAN",
		/* [87] */
		"EDDIE ALBERT",
		/* [88] */
		"EDDIE MURPHY",
		/* [89] */
		"EDGAR BUCHANAN",
		/* [90] */
		"EDWARD G. ROBINSON",
		/* [91] */
		"ELLEN BARKIN",
		/* [92] */
		"ELLIOTT GOULD",
		/* [93] */
		"ELVIRA",
		/* [94] */
		"EMELIO ESTEVEZ",
		/* [95] */
		"EUGENE LEVY",
		/* [96] */
		"EVA MARIE SAINT",
		/* [97] */
		"FABIAN",
		/* [98] */
		"FARRAH FAWCETT",
		/* [99] */
		"FOREST TUCKER",
		/* [100] */
		"FRANCES LANGFORD",
		/* [101] */
		"FRANCIS BAVIER",
		/* [102] */
		"FRANK MORGAN",
		/* [103] */
		"FRED GWINN",
		/* [104] */
		"FRED MACMURRAY",
		/* [105] */
		"FRED WILLIAMSON",
		/* [106] */
		"GARY BURGHOFF",
		/* [107] */
		"GARY COLLINS",
		/* [108] */
		"GAVIN MACLEOD",
		/* [109] */
		"GENE KRUPA",
		/* [110] */
		"GENE WILDER",
		/* [111] */
		"GEORGE LINDSEY",
		/* [112] */
		"GEORGE NOBEL",
		/* [113] */
		"GEORGE SEGAL",
		/* [114] */
		"GEORGE TAKEI",
		/* [115] */
		"GEORGE TOBIAS BARTON MACLANE",
		/* [116] */
		"GEORGE WENDT",
		/* [117] */
		"GRACE JONES",
		/* [118] */
		"GRACE KELLY",
		/* [119] */
		"HAROLD RAMIS",
		/* [120] */
		"HARRISON FORD",
		/* [121] */
		"HARRY ANDERSON",
		/* [122] */
		"HARRY CONNICK JR.",
		/* [123] */
		"HARRY DEAN STANTON",
		/* [124] */
		"HARRY MORGAN",
		/* [125] */
		"HENNY YOUNGMAN",
		/* [126] */
		"HENRY FONDA",
		/* [127] */
		"HENRY MORGAN",
		/* [128] */
		"HENRY WINKLER",
		/* [129] */
		"HOLLY HUNTER",
		/* [130] */
		"HOWARD HESSEMAN",
		/* [131] */
		"HOWARD MCNEAR",
		/* [132] */
		"HOWARD MORRIS",
		/* [133] */
		"HUGH MARLOWE",
		/* [134] */
		"HUMPHREY BOGART",
		/* [135] */
		"INGRID BERGMAN",
		/* [136] */
		"IRENE CARA",
		/* [137] */
		"JACK DODSON",
		/* [138] */
		"JACK HALEY",
		/* [139] */
		"JACK LEMMEN",
		/* [140] */
		"JACK NICHOLSON",
		/* [141] */
		"JACK PALANCE",
		/* [142] */
		"JACKIE CHAN",
		/* [143] */
		"JACKIE COOGAN",
		/* [144] */
		"JACKIE MASON",
		/* [145] */
		"JAMES BROWN",
		/* [146] */
		"JAMES COCO",
		/* [147] */
		"JAMES DEAN",
		/* [148] */
		"JAMES DOOHAN",
		/* [149] */
		"JAMES STEWART",
		/* [150] */
		"JAMIE FARR",
		/* [151] */
		"JANE SEYMOUR",
		/* [152] */
		"JANET LEIGH",
		/* [153] */
		"JASON ROBARDS",
		/* [154] */
		"JEFF BRIDGES",
		/* [155] */
		"JEFF GOLDBLUM",
		/* [156] */
		"JENNIFER GREY",
		/* [157] */
		"JENNIFER JASON LEIGH",
		/* [158] */
		"JERRY VAN DYKE",
		/* [159] */
		"JILL CLAYBURGH",
		/* [160] */
		"JILL EIKENBERRY",
		/* [161] */
		"JIM BACKUS",
		/* [162] */
		"JIM NABORS",
		/* [163] */
		"JOE FLAHERTY",
		/* [164] */
		"JOHN AGAR",
		/* [165] */
		"JOHN ASTIN",
		/* [166] */
		"JOHN BELUSHI",
		/* [167] */
		"JOHN CANDY",
		/* [168] */
		"JOHN GARFIELD",
		/* [169] */
		"JOHN GOODMAN",
		/* [170] */
		"JOHN HILLERMAN",
		/* [171] */
		"JOHN LARROQUETTE",
		/* [172] */
		"JOHN LITHGOW",
		/* [173] */
		"JOHN PARAGON",
		/* [174] */
		"JOHN TURTURRO",
		/* [175] */
		"JOHN WAYNE",
		/* [176] */
		"JOHNNY DEPP",
		/* [177] */
		"JUDGE REINHOLD",
		/* [178] */
		"JUDY GARLAND",
		/* [179] */
		"JULIE ANDREWS",
		/* [180] */
		"JULIE HAGERTY",
		/* [181] */
		"JULIE MONTGOMERY",
		/* [182] */
		"JUNE ALLYSON",
		/* [183] */
		"KAREEM ABDUL JABAAR",
		/* [184] */
		"KARL MAULDEN",
		/* [185] */
		"KELLY LEBROCK",
		/* [186] */
		"KENNETH MARS",
		/* [187] */
		"KETHARINE HEPBURN",
		/* [188] */
		"KEVIN BACON",
		/* [189] */
		"KEVIN MCCARTHY",
		/* [190] */
		"KIM BASINGER",
		/* [191] */
		"KIM CATTRALL",
		/* [192] */
		"KIRSTIE ALLEY",
		/* [193] */
		"KRIS KRISTOFFERSON",
		/* [194] */
		"LANA TURNER",
		/* [195] */
		"LARRY, MOE, AND CURLY",
		/* [196] */
		"LAUREN HUTTON",
		/* [197] */
		"LAWRENCE MONOSON",
		/* [198] */
		"LEONARD NIMOY",
		/* [199] */
		"LESLIE NIELSON",
		/* [200] */
		"LEVON HELM",
		/* [201] */
		"LIONEL BARRYMORE",
		/* [202] */
		"LIZA MINELLI",
		/* [203] */
		"LLOYD BRIDGES",
		/* [204] */
		"LONI ANDERSON",
		/* [205] */
		"LOUIS ARMSTRONG",
		/* [206] */
		"MACKENZIE PHILLIPS",
		/* [207] */
		"MADELINE KAHN",
		/* [208] */
		"MADONNA",
		/* [209] */
		"MAKO",
		/* [210] */
		"MARILYN MONROE",
		/* [211] */
		"MARLON BRANDO",
		/* [212] */
		"MARSHA HUNT",
		/* [213] */
		"MARTIN MULL",
		/* [214] */
		"MARTY FELDMAN",
		/* [215] */
		"MARY STEENBURGEN",
		/* [216] */
		"MATT DILLON",
		/* [217] */
		"MATT FREWER",
		/* [218] */
		"MATTHEW BRODERICK",
		/* [219] */
		"MAUREEN O'HARA",
		/* [220] */
		"MAXWELL CAULFIELD",
		/* [221] */
		"MEL BLANC",
		/* [222] */
		"MEL GIBSON",
		/* [223] */
		"MEL TILLIS",
		/* [224] */
		"MICHAEL CAINE",
		/* [225] */
		"MICHAEL GOUGH",
		/* [226] */
		"MICHAEL J. FOX",
		/* [227] */
		"MICHAEL KEATON",
		/* [228] */
		"MICHAEL KEITH",
		/* [229] */
		"MICHELLE PFIEFFER",
		/* [230] */
		"MICKEY ROURKE",
		/* [231] */
		"MOLLY RINGWALD",
		/* [232] */
		"NANCY WALKER",
		/* [233] */
		"NATALE WOOD",
		/* [234] */
		"NATASHA KINSKI",
		/* [235] */
		"NATELIE WOOD",
		/* [236] */
		"NED BEATTY",
		/* [237] */
		"NICHELLE NICHOLS",
		/* [238] */
		"NICHOLAS CAGE",
		/* [239] */
		"NIGEL BRUCE",
		/* [240] */
		"OLIVER HARDY",
		/* [241] */
		"PAT HINGLE",
		/* [242] */
		"PAUL ANKA",
		/* [243] */
		"PAUL DOOLEY",
		/* [244] */
		"PAUL NEWMAN",
		/* [245] */
		"PAUL REISER",
		/* [246] */
		"PAUL REUBENS",
		/* [247] */
		"PAUL SIMON",
		/* [248] */
		"PEE-WEE HERMAN",
		/* [249] */
		"PETER BOYLE",
		/* [250] */
		"PETER COYOTE",
		/* [251] */
		"PETER FALK",
		/* [252] */
		"PETER FONDA",
		/* [253] */
		"PETER GALLAGHER",
		/* [254] */
		"PETER GRAVES",
		/* [255] */
		"PETER LAWFORD",
		/* [256] */
		"PETER LORRE",
		/* [257] */
		"PETER O'TOOLE",
		/* [258] */
		"PETER SELLERS",
		/* [259] */
		"PETER WELLER",
		/* [260] */
		"PHOEBE CATES",
		/* [261] */
		"RACHEL WARD",
		/* [262] */
		"RANCE HOWARD",
		/* [263] */
		"RANDY QUAID",
		/* [264] */
		"RAY BOLGER",
		/* [265] */
		"RAY CHARLES",
		/* [266] */
		"RAY WALSTON",
		/* [267] */
		"RAYMOND BURR",
		/* [268] */
		"REBECCA DEMORNAY",
		/* [269] */
		"RED BUTTONS",
		/* [270] */
		"REIHOLD WEEGE",
		/* [271] */
		"RICARDO MONTALBAN",
		/* [272] */
		"RICHARD BELZER",
		/* [273] */
		"RICHARD BURTON",
		/* [274] */
		"RICHARD DAWSON",
		/* [275] */
		"RICHARD DREYFUSS",
		/* [276] */
		"RICHARD PRYOR",
		/* [277] */
		"RICHARD ROUNDTREE",
		/* [278] */
		"RICK MORANIS",
		/* [279] */
		"RIP TORN",
		/* [280] */
		"ROBERT CARRADINE",
		/* [281] */
		"ROBERT DUBALL",
		/* [282] */
		"ROBERT ENGLUND",
		/* [283] */
		"ROBERT MITCHUM",
		/* [284] */
		"ROBERT REDFORD",
		/* [285] */
		"ROBERT STACK",
		/* [286] */
		"ROBERT WAGNER",
		/* [287] */
		"ROBERT WUHL",
		/* [288] */
		"ROBRET HAYES",
		/* [289] */
		"ROD STEIGER",
		/* [290] */
		"RODDY MCDOWALL",
		/* [291] */
		"RODNEY DANGERFIELD",
		/* [292] */
		"RODNEY DILLARD",
		/* [293] */
		"ROGER MOORE",
		/* [294] */
		"RON HOWARD",
		/* [295] */
		"ROSANNE ARQUETTE",
		/* [296] */
		"ROSEMARY CLOONEY",
		/* [297] */
		"ROY SCHEIDER",
		/* [298] */
		"SAL MINEO",
		/* [299] */
		"SALLY KELLERMAN",
		/* [300] */
		"SAMMY DAVIS JR.",
		/* [301] */
		"SCATMAN CROTHERS",
		/* [302] */
		"SCOTT BAIO",
		/* [303] */
		"SEAN CONNERY",
		/* [304] */
		"SEAN PENN",
		/* [305] */
		"SHELLEY LONG",
		/* [306] */
		"SHIRLEY MACLAINE",
		/* [307] */
		"SIGOURNEY WEAVER",
		/* [308] */
		"SIR JOHN GIELGUG",
		/* [309] */
		"SOH YAMAMURA",
		/* [310] */
		"SPENCER TRACY",
		/* [311] */
		"STANLEY LAUREL",
		/* [312] */
		"STEPHEN FURST",
		/* [313] */
		"STEVE GUTENBURG",
		/* [314] */
		"STEVE LAWRENCE",
		/* [315] */
		"STEVE MARTIN",
		/* [316] */
		"STING",
		/* [317] */
		"SUSAN SARANDON",
		/* [318] */
		"SUZANNE SOMERS",
		/* [319] */
		"TAWNY KITAEN",
		/* [320] */
		"TED KNIGHT",
		/* [321] */
		"TERENCE HILL",
		/* [322] */
		"TERI GARR",
		/* [323] */
		"TERRY BRADSHAW",
		/* [324] */
		"THE RAMONES",
		/* [325] */
		"TIM CONWAY",
		/* [326] */
		"TIM MATHESON",
		/* [327] */
		"TIMOTHY BOTTOMS",
		/* [328] */
		"TOM CRUISE",
		/* [329] */
		"TOM HANKS",
		/* [330] */
		"TOM SCOTT",
		/* [331] */
		"TOM SELLECK",
		/* [332] */
		"TOM SKERRITT",
		/* [333] */
		"TOM WAITS",
		/* [334] */
		"TOMMY EWELL",
		/* [335] */
		"TONY CURTIS",
		/* [336] */
		"TOTO",
		/* [337] */
		"TRACY SMITH",
		/* [338] */
		"TRUMAN COPOTE",
		/* [339] */
		"VALERIA GOLINO",
		/* [340] */
		"VALERIE QUENNESSEN",
		/* [341] */
		"VIRGINIA MADSEN",
		/* [342] */
		"W.C. FIELDS",
		/* [343] */
		"WALTER KOENIG",
		/* [344] */
		"WILFORD BRIMLEY",
		/* [345] */
		"WILLIAM CHRISTOPHER",
		/* [346] */
		"WILLIAM FORSYTHE",
		/* [347] */
		"WILLIAM SHATNER",
		/* [348] */
		"WILLIE AAMES",
		/* [349] */
		"WOLFMAN JACK",
		/* [350] */
		"YVONNE DE CARLO"
	}
};

resource 'PUZZ' (130, "Song", purgeable) {
	{	/* array StringArray: 230 elements */
		/* [1] */
		"A DAY IN THE LIFE",
		/* [2] */
		"A PASSAGE TO BANGKOK",
		/* [3] */
		"AIN'T TALKIN' 'BOUT LOVE",
		/* [4] */
		"AJA",
		/* [5] */
		"ALL DAY AND ALL OF THE NIGHT",
		/* [6] */
		"ALL YOU ZOMBIES",
		/* [7] */
		"AMERICAN PIE",
		/* [8] */
		"AND THE CRADLE WILL ROCK",
		/* [9] */
		"ANOTHER ONE BITES THE DUST",
		/* [10] */
		"ANTHEM",
		/* [11] */
		"ATOMIC PUNK",
		/* [12] */
		"BACK IN BLACK",
		/* [13] */
		"BAD BOY",
		/* [14] */
		"BASTILLE DAY",
		/* [15] */
		"BE STIFF",
		/* [16] */
		"BEAUTIFUL GIRLS",
		/* [17] */
		"BEHIND MY CAMEL",
		/* [18] */
		"BEST I CAN",
		/* [19] */
		"BIG BAD BILL",
		/* [20] */
		"BIG MONEY",
		/* [21] */
		"BOHEMIAN RHAPSODY",
		/* [22] */
		"BOMBS AWAY",
		/* [23] */
		"BORN IN THE FIFTIES",
		/* [24] */
		"BOTTOMS UP!",
		/* [25] */
		"BRING ON THE NIGHT",
		/* [26] */
		"BUNGALOW BILL",
		/* [27] */
		"BUTT TOWN",
		/* [28] */
		"BY-TOR AND THE SNOW DOG",
		/* [29] */
		"CAN'T STAND LOSING YOU",
		/* [30] */
		"CANARY IN A COALMINE",
		/* [31] */
		"CATCH ME NOW I'M FALLING",
		/* [32] */
		"CATHEDRAL",
		/* [33] */
		"CELLULOID HEROES",
		/* [34] */
		"CHEMISTRY",
		/* [35] */
		"CLOSER TO THE HEART",
		/* [36] */
		"CONTACT",
		/* [37] */
		"COULD THIS BE MAGIC?",
		/* [38] */
		"COUNTDOWN",
		/* [39] */
		"CRAZY LITTLE THING CALLED LOVE",
		/* [40] */
		"CULT OF PERSONALITY",
		/* [41] */
		"CYGNUS X-1",
		/* [42] */
		"D.O.A.",
		/* [43] */
		"DANCE THE NIGHT AWAY",
		/* [44] */
		"DANCING IN THE STREET",
		/* [45] */
		"DARKNESS",
		/* [46] */
		"DAVID WATTS",
		/* [47] */
		"DE DO DO DO, DE DA DA DA",
		/* [48] */
		"DEACON BLUES",
		/* [49] */
		"DEATHWISH",
		/* [50] */
		"DEMOLITION MAN",
		/* [51] */
		"DIFFERENT STRINGS",
		/* [52] */
		"DIGITAL MAN",
		/* [53] */
		"DIRTY DEEDS DONE DIRT CHEAP",
		/* [54] */
		"DIRTY MOVIES",
		/* [55] */
		"DISTANT EARLY WARNING",
		/* [56] */
		"DO WA DIDDY",
		/* [57] */
		"DO YA WANNA DANCE?",
		/* [58] */
		"DOES EVERYBODY STARE",
		/* [59] */
		"DON'T GET AROUND MUCH ANYMORE",
		/* [60] */
		"DON'T STAND SO CLOSE TO ME",
		/* [61] */
		"DOWN UNDER",
		/* [62] */
		"DRIVEN TO TEARS",
		/* [63] */
		"DUST IN THE WIND",
		/* [64] */
		"ENTRE NOUS",
		/* [65] */
		"ERUPTION",
		/* [66] */
		"EVERY BREATH YOU TAKE",
		/* [67] */
		"EVERY LITTLE THING SHE DOES IS MAGIC",
		/* [68] */
		"EVERYBODY WANTS SOME!!",
		/* [69] */
		"FAT BOTTOMED GIRLS",
		/* [70] */
		"FEEL YOUR LOVE TONIGHT",
		/* [71] */
		"FINISH WHAT YA STARTED",
		/* [72] */
		"FOOLS",
		/* [73] */
		"FREE WILL",
		/* [74] */
		"GET BACK",
		/* [75] */
		"GIRL U WANT",
		/* [76] */
		"GUITARS, CADILLACS",
		/* [77] */
		"HANG 'EM HIGH",
		/* [78] */
		"HAPPY TRAILS",
		/* [79] */
		"HEAR ABOUT IT LATER",
		/* [80] */
		"HELL'S BELLS",
		/* [81] */
		"HEY JOE",
		/* [82] */
		"HEY JUDE",
		/* [83] */
		"HOLE IN MY LIFE",
		/* [84] */
		"HOMO JOCKO",
		/* [85] */
		"HOTEL CALIFORNIA",
		/* [86] */
		"HUNGRY FOR YOU",
		/* [87] */
		"I THINK I'M GOING BALD",
		/* [88] */
		"I'M IN LOVE WITH MY CAR",
		/* [89] */
		"I'M THE ONE",
		/* [90] */
		"ICE CREAM MAN",
		/* [91] */
		"IN A SIMPLE RHYME",
		/* [92] */
		"INTRUDER",
		/* [93] */
		"INVISIBLE SUN",
		/* [94] */
		"IS SHE REALLY GOING OUT WITH HIM?",
		/* [95] */
		"IT HAD TO BE YOU",
		/* [96] */
		"IT'S ALRIGHT FOR YOU",
		/* [97] */
		"JACOB'S LADDER",
		/* [98] */
		"JAMIE'S CRYIN'",
		/* [99] */
		"JOHN, I'M ONLY DANCING",
		/* [100] */
		"KING OF PAIN",
		/* [101] */
		"LA VILLA STRANGIATO",
		/* [102] */
		"LAKESIDE PARK",
		/* [103] */
		"LET'S DANCE",
		/* [104] */
		"LIFE ON MARS?",
		/* [105] */
		"LIFE'S BEEN GOOD",
		/* [106] */
		"LIGHT UP THE SKY",
		/* [107] */
		"LIME LIGHT",
		/* [108] */
		"LITTLE DREAMER",
		/* [109] */
		"LITTLE GUITARS",
		/* [110] */
		"LITTLE WING",
		/* [111] */
		"LIVE AND LET DIE",
		/* [112] */
		"LOLA",
		/* [113] */
		"LOSS OF CONTROL",
		/* [114] */
		"LOVE REARS ITS UGLY HEAD",
		/* [115] */
		"LOVELY RITA",
		/* [116] */
		"LOW BUDGET",
		/* [117] */
		"MAIN STREET",
		/* [118] */
		"MAKE ME SAD",
		/* [119] */
		"MAN IN A SUITCASE",
		/* [120] */
		"MASOKO TANGA",
		/* [121] */
		"MEAN STREET",
		/* [122] */
		"MESSAGE IN A BOTTLE",
		/* [123] */
		"MISS GRADENKO",
		/* [124] */
		"MOTHER",
		/* [125] */
		"MURDER BY NUMBERS",
		/* [126] */
		"NATURAL SCIENCE",
		/* [127] */
		"NEW WORLD MAN",
		/* [128] */
		"NEXT TO YOU",
		/* [129] */
		"NO TIME THIS TIME",
		/* [130] */
		"O MY GOD",
		/* [131] */
		"OH! PRETTY WOMAN",
		/* [132] */
		"OMEGAMAN",
		/* [133] */
		"ON ANY OTHER DAY",
		/* [134] */
		"ON FIRE",
		/* [135] */
		"ONE FOOT OUT THE DOOR",
		/* [136] */
		"ONE WORLD (NOT THREE)",
		/* [137] */
		"OUTTA LOVE AGAIN",
		/* [138] */
		"PANAMA",
		/* [139] */
		"PANIC IN DETROIT",
		/* [140] */
		"PLANET EARTH",
		/* [141] */
		"PLEASE, PLEASE ME",
		/* [142] */
		"POINT OF NO RETURN",
		/* [143] */
		"PORGY AND BESS",
		/* [144] */
		"PRESSURE",
		/* [145] */
		"PRESSURE DROP",
		/* [146] */
		"PRESTO",
		/* [147] */
		"PROUD MARY",
		/* [148] */
		"PURPLE HAZE",
		/* [149] */
		"PUSH COMES TO SHOVE",
		/* [150] */
		"REBEL REBEL",
		/* [151] */
		"RED BARCHETTA",
		/* [152] */
		"REGGATTA DE BLANC",
		/* [153] */
		"REHUMANIZE YOURSELF",
		/* [154] */
		"REVOLUTION",
		/* [155] */
		"RHAPSODY IN BLUE",
		/* [156] */
		"RIKI DON'T LOSE THAT NUMBER",
		/* [157] */
		"ROCK AROUND THE CLOCK",
		/* [158] */
		"ROMEO DELIGHT",
		/* [159] */
		"ROXANNE",
		/* [160] */
		"RUNNIN' WITH THE DEVIL",
		/* [161] */
		"SECRET JOURNEY",
		/* [162] */
		"SECRETS",
		/* [163] */
		"SHADOWS IN THE RAIN",
		/* [164] */
		"SHEER HEART ATTACK",
		/* [165] */
		"SHOULD I STAY OR SHOULD I GO?",
		/* [166] */
		"SHOW DON'T TELL",
		/* [167] */
		"SINNER'S SWING!",
		/* [168] */
		"SMELLS LIKE NIRVANA",
		/* [169] */
		"SMELLS LIKE TEEN SPIRIT",
		/* [170] */
		"SMOKE ON THE WATER",
		/* [171] */
		"SO LONELY",
		/* [172] */
		"SO THIS IS LOVE?",
		/* [173] */
		"SOMEBODY GET ME A DOCTOR",
		/* [174] */
		"SOMETHING FOR NOTHING",
		/* [175] */
		"SPACE TRUCKIN'",
		/* [176] */
		"SPANISH FLY",
		/* [177] */
		"SPIRITS IN THE MATERIAL WORLD",
		/* [178] */
		"STOMPIN' AT THE SAVOY",
		/* [179] */
		"STRING OF PEARLS",
		/* [180] */
		"SUBDIVISIONS",
		/* [181] */
		"SUFFRAGETTE CITY",
		/* [182] */
		"SUNDAY AFTERNOON IN THE PARK",
		/* [183] */
		"SUNGLASSES AT NIGHT",
		/* [184] */
		"SUPERCONDUCTOR",
		/* [185] */
		"SWEET EMOTION",
		/* [186] */
		"SWEET LEAF",
		/* [187] */
		"SYNCHRONICITY",
		/* [188] */
		"TAKE YOUR WHISKY HOME",
		/* [189] */
		"TEA IN THE SAHARA",
		/* [190] */
		"TEMPLES OF SYRINX",
		/* [191] */
		"THE ANALOG KID",
		/* [192] */
		"THE BANANA BOAT SONG",
		/* [193] */
		"THE BED'S TOO BIG WITHOUT YOU",
		/* [194] */
		"THE CAMERA EYE",
		/* [195] */
		"THE FULL BUG",
		/* [196] */
		"THE OTHER WAY OF STOPPING",
		/* [197] */
		"THE SHOW MUST GO ON",
		/* [198] */
		"THE SPIRIT OF RADIO",
		/* [199] */
		"THE TREES",
		/* [200] */
		"THE TWILIGHT ZONE",
		/* [201] */
		"THERE WAS AN OLD LADY",
		/* [202] */
		"THROUGH BEING COOL",
		/* [203] */
		"TOM SAWYER",
		/* [204] */
		"TOO MUCH INFORMATION",
		/* [205] */
		"TORA! TORA!",
		/* [206] */
		"TOWN CALLED MALICE",
		/* [207] */
		"UNCHAINED",
		/* [208] */
		"UNDER PRESSURE",
		/* [209] */
		"VITAL SIGNS",
		/* [210] */
		"VOICES IN MY HEAD",
		/* [211] */
		"WALK THIS WAY",
		/* [212] */
		"WALKING IN YOUR FOOTSTEPS",
		/* [213] */
		"WALKING ON THE MOON",
		/* [214] */
		"WHEN I'M SIXTY-FOUR",
		/* [215] */
		"WHERE HAVE ALL THE GOOD TIMES GONE",
		/* [216] */
		"WHILE MY GUITAR GENTLY WEEPS",
		/* [217] */
		"WHIP IT",
		/* [218] */
		"WHITE CHRISTMAS",
		/* [219] */
		"WHO WANTS TO LIVE FOREVER?",
		/* [220] */
		"WITCH HUNT",
		/* [221] */
		"WOMEN IN LOVE",
		/* [222] */
		"WORKING MAN",
		/* [223] */
		"WRAPPED AROUND YOUR FINGER",
		/* [224] */
		"XANADU",
		/* [225] */
		"YOU REALLY GOT ME",
		/* [226] */
		"YOU'RE BREAKING MY HEART",
		/* [227] */
		"YOU'RE NO GOOD",
		/* [228] */
		"YOUNG AMERICANS",
		/* [229] */
		"YYZ",
		/* [230] */
		"ZIGGY STARDUST"
	}
};

resource 'PUZZ' (131, "TV Show or Cartoon", purgeable) {
	{	/* array StringArray: 174 elements */
		/* [1] */
		"A CHARLIE BROWN CHRISTMAS",
		/* [2] */
		"ABC'S WIDE WORLD OF SPORTS",
		/* [3] */
		"ADAM TWELVE",
		/* [4] */
		"ALFRED HITCHCOCK PRESENTS",
		/* [5] */
		"AMERICAN BANDSTAND",
		/* [6] */
		"BARETTA",
		/* [7] */
		"BARNABY JONES",
		/* [8] */
		"BARNEY MILLER",
		/* [9] */
		"BAT MASTERSON",
		/* [10] */
		"BATMAN",
		/* [11] */
		"BEN CASEY",
		/* [12] */
		"BEWITCHED",
		/* [13] */
		"BONANZA",
		/* [14] */
		"BRANDED",
		/* [15] */
		"CAPTAIN KANGAROO",
		/* [16] */
		"CAR FIFTY-FOUR, WHERE ARE YOU?",
		/* [17] */
		"CASPER, THE FRIENDLY GHOST",
		/* [18] */
		"CHARLIE'S ANGELS",
		/* [19] */
		"CHEERS",
		/* [20] */
		"COMBAT",
		/* [21] */
		"DAKTARI",
		/* [22] */
		"DARK SHADOWS",
		/* [23] */
		"DASTARDLY AND MUTTLEY",
		/* [24] */
		"DENNIS THE MENACE",
		/* [25] */
		"DOBIE GILLIS",
		/* [26] */
		"DRAGNET",
		/* [27] */
		"ENTERTAINMENT TONIGHT",
		/* [28] */
		"F TROOP",
		/* [29] */
		"FAT ALBERT AND THE COSBY KIDS",
		/* [30] */
		"FELIX THE CAT",
		/* [31] */
		"FIREBALL XL-5",
		/* [32] */
		"FLIPPER",
		/* [33] */
		"FROSTY THE SNOWMAN",
		/* [34] */
		"GET SMART",
		/* [35] */
		"GIDGET",
		/* [36] */
		"GILLIGAN'S ISLAND",
		/* [37] */
		"GOMER PYLE, U.S.M.C.",
		/* [38] */
		"GREEN ACRES",
		/* [39] */
		"HAPPY DAYS",
		/* [40] */
		"HAPPY TRAILS",
		/* [41] */
		"HAROLD AND HIS PURPLE CRAYON",
		/* [42] */
		"HAVE GUN WILL TRAVEL",
		/* [43] */
		"HAWAII FIVE-O",
		/* [44] */
		"HAWAIIAN EYE",
		/* [45] */
		"HILL STREET BLUES",
		/* [46] */
		"HOGAN'S HEROES",
		/* [47] */
		"HOW THE GRINCH STOLE CHRISTMAS",
		/* [48] */
		"HOWDY DOODY",
		/* [49] */
		"HUCKLEBERRY HOUND",
		/* [50] */
		"I DREAM OF JEANNIE",
		/* [51] */
		"I LOVE LUCY",
		/* [52] */
		"I MARRIED JOAN",
		/* [53] */
		"I SPY",
		/* [54] */
		"IRONSIDE",
		/* [55] */
		"IT'S THE GREAT PUMPKIN CHARLIE BROWN",
		/* [56] */
		"IT'S YOUR FIRST KISS CHARLIE BROWN",
		/* [57] */
		"JEOPARDY",
		/* [58] */
		"JONNY QUEST",
		/* [59] */
		"JOSIE AND THE PUSSYCATS",
		/* [60] */
		"KOJAK",
		/* [61] */
		"L.A. LAW",
		/* [62] */
		"LATE NIGHT WITH DAVID LETTERMAN",
		/* [63] */
		"LAVERNE AND SHIRLEY",
		/* [64] */
		"LEAVE IT TO BEAVER",
		/* [65] */
		"LOONEY TUNES",
		/* [66] */
		"LOST IN SPACE",
		/* [67] */
		"LOVE BOAT",
		/* [68] */
		"LOVE, AMERICAN STYLE",
		/* [69] */
		"MAGILLA GORILLA",
		/* [70] */
		"MAGNUM, P.I.",
		/* [71] */
		"MANNIX",
		/* [72] */
		"MARCUS WELBY, M.D.",
		/* [73] */
		"MARY TYLER MOORE",
		/* [74] */
		"MASH",
		/* [75] */
		"MAUDE",
		/* [76] */
		"MAVERICK",
		/* [77] */
		"MCHALE'S NAVY",
		/* [78] */
		"MEDICAL CENTER",
		/* [79] */
		"MERRIE MELODIES",
		/* [80] */
		"MIAMI VICE",
		/* [81] */
		"MIGHTY MOUSE - THE NEW ADVENTURES",
		/* [82] */
		"MISSION IMPOSSIBLE",
		/* [83] */
		"MONTY PYTHON'S FLYING CIRCUS",
		/* [84] */
		"MR. ED",
		/* [85] */
		"MR. MAGOO",
		/* [86] */
		"MY FAVORITE MARTIAN",
		/* [87] */
		"MY MOTHER THE CAR",
		/* [88] */
		"MY THREE SONS",
		/* [89] */
		"NIGHT COURT",
		/* [90] */
		"NORTHERN EXPOSURE",
		/* [91] */
		"ONE DAY AT A TIME",
		/* [92] */
		"OUTER LIMITS",
		/* [93] */
		"PEE-WEE'S PLAYHOUSE",
		/* [94] */
		"PERRY MASON",
		/* [95] */
		"PETER GUNN",
		/* [96] */
		"PETTICOAT JUNCTION",
		/* [97] */
		"POPEYE",
		/* [98] */
		"QUINCY, M.E.",
		/* [99] */
		"RAT PATROL",
		/* [100] */
		"RAWHIDE",
		/* [101] */
		"RIN TIN TIN",
		/* [102] */
		"ROCKFORD FILES",
		/* [103] */
		"ROUTE SIXTY-SIX",
		/* [104] */
		"RUDOLPH THE RED-NOSED REINDEER",
		/* [105] */
		"SANFORD AND SON",
		/* [106] */
		"SATURDAY NIGHT LIVE",
		/* [107] */
		"SCOOBY-DOO",
		/* [108] */
		"SEA HUNT",
		/* [109] */
		"SECRET AGENT MAN",
		/* [110] */
		"SEVENTY-SEVEN SUNSET STRIP",
		/* [111] */
		"SMOTHERS BROTHERS COMEDY HOUR",
		/* [112] */
		"SPEED RACER",
		/* [113] */
		"STAR TREK",
		/* [114] */
		"STARSKY AND HUTCH",
		/* [115] */
		"THE STREETS OF SAN FRANCISCO",
		/* [116] */
		"SUPERMAN",
		/* [117] */
		"SURFSIDE SIX",
		/* [118] */
		"SWAT",
		/* [119] */
		"TARZAN",
		/* [120] */
		"THAT GIRL",
		/* [121] */
		"THE A-TEAM",
		/* [122] */
		"THE ADDAMS FAMILY",
		/* [123] */
		"THE ADVENTURES OF ROBIN HOOD",
		/* [124] */
		"THE ANDY GRIFFITH SHOW",
		/* [125] */
		"THE ARCHIES",
		/* [126] */
		"THE AVENGERS",
		/* [127] */
		"THE BEVERLY HILLBILLIES",
		/* [128] */
		"THE BRADY BUNCH",
		/* [129] */
		"THE COURTSHIP OF EDDIE'S FATHER",
		/* [130] */
		"THE DICK VAN DYKE SHOW",
		/* [131] */
		"THE DONNA REED SHOW",
		/* [132] */
		"THE EQUALIZER",
		/* [133] */
		"THE F.B.I.",
		/* [134] */
		"THE FLINTSTONES",
		/* [135] */
		"THE GREEN HORNET",
		/* [136] */
		"THE HONEYMOONERS",
		/* [137] */
		"THE JACKIE GLEASON SHOW",
		/* [138] */
		"THE JETSONS",
		/* [139] */
		"THE LATE LATE LATE SHOW",
		/* [140] */
		"THE LITTLE RASCALS",
		/* [141] */
		"THE LONE RANGER",
		/* [142] */
		"THE MAN FROM U.N.C.L.E.",
		/* [143] */
		"THE MOD SQUAD",
		/* [144] */
		"THE MONKEES",
		/* [145] */
		"THE MUNSTERS",
		/* [146] */
		"THE NBC MYSTERY MOVIE",
		/* [147] */
		"THE ODD COUPLE",
		/* [148] */
		"THE PARTRIDGE FAMILY",
		/* [149] */
		"THE PATTY DUKE SHOW",
		/* [150] */
		"THE PINK PANTHER",
		/* [151] */
		"THE REBEL",
		/* [152] */
		"THE REN AND STIMPY SHOW",
		/* [153] */
		"THE RIFLEMAN",
		/* [154] */
		"THE ROCKY AND BULLWINKLE SHOW",
		/* [155] */
		"THE ROOKIES",
		/* [156] */
		"THE SAINT",
		/* [157] */
		"THE THREE STOOGES",
		/* [158] */
		"THE TONIGHT SHOW",
		/* [159] */
		"THE TWILIGHT ZONE",
		/* [160] */
		"THE VIRGINIAN",
		/* [161] */
		"THE WILD WILD WEST",
		/* [162] */
		"THE WOODY WOODPECKER SHOW",
		/* [163] */
		"THREE'S COMPANY",
		/* [164] */
		"TIME TUNNEL",
		/* [165] */
		"TOP CAT",
		/* [166] */
		"TWELVE O'CLOCK HIGH",
		/* [167] */
		"TWIN PEAKS",
		/* [168] */
		"UNDERDOG",
		/* [169] */
		"VOYAGE TO THE BOTTOM OF THE SEA",
		/* [170] */
		"WAGON TRIAN",
		/* [171] */
		"WELCOME BACK, KOTTER",
		/* [172] */
		"WKRP IN CINCINNATI",
		/* [173] */
		"WONDER WOMAN",
		/* [174] */
		"YOGI BEAR"
	}
};

resource 'PUZZ' (132, "Thing", purgeable) {
	{	/* array StringArray: 56 elements */
		/* [1] */
		"ABOVE-GROUND SWIMMING POOL",
		/* [2] */
		"ANSWERING MACHINE",
		/* [3] */
		"ALFALFA BEAN SPROUT",
		/* [4] */
		"AMETHYST RING",
		/* [5] */
		"AUGMENTED MINOR SEVENTH CHORD",
		/* [6] */
		"AUTOMATIC DISHWASHER",
		/* [7] */
		"AFRICAN PYTHON",
		/* [8] */
		"BAR-BELLS AND DUMB-BELLS",
		/* [9] */
		"BUTTERFLY CHAIR",
		/* [10] */
		"BED-SIDE TABLE",
		/* [11] */
		"BLACK OLIVE",
		/* [12] */
		"CAN OPENER",
		/* [13] */
		"CAP PISTOL",
		/* [14] */
		"CELLULAR PHONE",
		/* [15] */
		"CHEVROLET CORVETTE",
		/* [16] */
		"CIRCULAR SAW",
		/* [17] */
		"COCKER SPANIEL",
		/* [18] */
		"COUSCOUS  ",
		/* [19] */
		"DIESEL-ELECTRIC LOCOMOTIVE",
		/* [20] */
		"DODGE VIPER",
		/* [21] */
		"FENDER STRATOCASTER",
		/* [22] */
		"FISH AND CHIPS",
		/* [23] */
		"FOUNTAIN PEN",
		/* [24] */
		"HAL TWO-THOUSAND",
		/* [25] */
		"HEWLETT PACKARD DESKWRITER",
		/* [26] */
		"HIGH-HEELED TENNIS SHOES",
		/* [27] */
		"HOT POTATO",
		/* [28] */
		"IMAGEWRITER II",
		/* [29] */
		"INDIANAPOLIS FIVE-HUNDRED",
		/* [30] */
		"JIGSAW PUZZLE",
		/* [31] */
		"LODGE MEETING",
		/* [32] */
		"MICROWAVE OVEN",
		/* [33] */
		"MOOSE ANTLERS",
		/* [34] */
		"MOUNTAIN BIKE",
		/* [35] */
		"NINETY-SIX-HUNDRED BAUD MODEM",
		/* [36] */
		"PEANUTS",
		/* [37] */
		"PEN AND INK",
		/* [38] */
		"PHILLIPS SCREWDRIVER",
		/* [39] */
		"PICCOLO SNARE DRUM",
		/* [40] */
		"PLYMOUTH BELVEDERE",
		/* [41] */
		"POPCORN POPPER",
		/* [42] */
		"RADIO TELESCOPE",
		/* [43] */
		"RADIOACTIVE WASTE",
		/* [44] */
		"REFRIGERATOR-FREEZER",
		/* [45] */
		"RUMBLE SEAT",
		/* [46] */
		"STROBE TUNER",
		/* [47] */
		"THE U.S.S. MINNOW",
		/* [48] */
		"THROW RUG",
		/* [49] */
		"TOUR DE FRANCE",
		/* [50] */
		"VCR",
		/* [51] */
		"VENETIAN BLIND",
		/* [52] */
		"WAFFLE IRON",
		/* [53] */
		"WALL-TO-WALL CARPETING",
		/* [54] */
		"WATER PISTOL",
		/* [55] */
		"WAVELESS WATERBED",
		/* [56] */
		"YO-YO"
	}
};

resource 'PUZZ' (133, "Place", purgeable) {
	{	/* array StringArray: 45 elements */
		/* [1] */
		"BATES MOTEL",
		/* [2] */
		"BATON ROUGE, LOUISIANA",
		/* [3] */
		"BEHIND THE COUCH",
		/* [4] */
		"BLACK HILLS, SOUTH DAKOTA",
		/* [5] */
		"BLACK OAK, ARKANSAS",
		/* [6] */
		"BOSTON, MASSACHUSETTS",
		/* [7] */
		"CICELY, ALASKA",
		/* [8] */
		"CITY OF INDUSTRY, CALIFORNIA",
		/* [9] */
		"CLEVELAND, OHIO",
		/* [10] */
		"DISTRICT OF COLUMBIA",
		/* [11] */
		"FARGO, NORTH DAKOTA",
		/* [12] */
		"FLAGSTAFF, ARIZONA",
		/* [13] */
		"GILLIGAN'S ISLAND",
		/* [14] */
		"GASOLINE ALLEY",
		/* [15] */
		"GLOVE COMPARTMENT",
		/* [16] */
		"HIROSHIMA, JAPAN",
		/* [17] */
		"HELSINKI, FINLAND",
		/* [18] */
		"IOWA CITY, IOWA",
		/* [19] */
		"JACKSON, MISSISSIPPI",
		/* [20] */
		"KEOTA, IOWA",
		/* [21] */
		"KEY WEST, FLORIDA",
		/* [22] */
		"KYOTO, JAPAN",
		/* [23] */
		"LAKE PLACID",
		/* [24] */
		"LAS VEGAS, NEVADA",
		/* [25] */
		"LITTLE BIG HORN",
		/* [26] */
		"LIVERPOOL, ENGLAND",
		/* [27] */
		"LOS ANGELES, CALIFORNIA",
		/* [28] */
		"MARTHA'S VINEYARD",
		/* [29] */
		"MAYBERRY, NORTH CAROLINA",
		/* [30] */
		"MIAMI, FLORIDA",
		/* [31] */
		"MY HOUSE",
		/* [32] */
		"NEW HAVEN, CONNECTICUT",
		/* [33] */
		"NO-MAN'S LAND",
		/* [34] */
		"ON TOP OF THE WORLD",
		/* [35] */
		"PEE-WEE'S PLAYHOUSE",
		/* [36] */
		"PROVIDENCE, RHODE ISLAND",
		/* [37] */
		"SEATTLE, WASHINGTON",
		/* [38] */
		"THE AMAZON BASIN",
		/* [39] */
		"THE EVERGLADES",
		/* [40] */
		"TREASURE ISLAND",
		/* [41] */
		"THE SWISS ALPS",
		/* [42] */
		"TIJUANA, MEXICO",
		/* [43] */
		"TWIN PEAKS, WASHINGTON",
		/* [44] */
		"WILLY WONKA'S CHOCOLATE FACTORY",
		/* [45] */
		"YELLOWSTONE NATIONAL PARK"
	}
};

resource 'PUZZ' (134, "Name", purgeable) {
	{	/* array StringArray: 71 elements */
		/* [1] */
		"ADOLF HITLER",
		/* [2] */
		"ALEXANDER THE GREAT",
		/* [3] */
		"ALFALFA, BUCKWHEAT, AND SPANKY",
		/* [4] */
		"ALFRED HITCHCOCK",
		/* [5] */
		"ANDROMEDA",
		/* [6] */
		"ANDY WORHOL",
		/* [7] */
		"AUNTIE EMM",
		/* [8] */
		"BART SIMPSON",
		/* [9] */
		"BING CROSBY",
		/* [10] */
		"BUD ABBOTT",
		/* [11] */
		"BORIS KARLOFF",
		/* [12] */
		"CALVIN AND HOBBES",
		/* [13] */
		"CHESTER CHEETAH",
		/* [14] */
		"COWBOYS AND INDIANS",
		/* [15] */
		"COPS AND ROBBERS",
		/* [16] */
		"COUNT DRACULA",
		/* [17] */
		"DAVID BOWIE",
		/* [18] */
		"DAVID LEE ROTH",
		/* [19] */
		"EARL THE DEAD CAT",
		/* [20] */
		"EDGAR ALLEN POE",
		/* [21] */
		"ERNEST HEMINGWAY",
		/* [22] */
		"GEORGE AND IRA GERSHWIN",
		/* [23] */
		"GREAT MID-WESTERN ICE CREAM COMPANY",
		/* [24] */
		"GENERAL GEORGE S. PATTON",
		/* [25] */
		"HER MAJESTY THE QUEEN",
		/* [26] */
		"HUCKLEBERRY FINN",
		/* [27] */
		"HOMER SIMPSON",
		/* [28] */
		"IRVING BERLIN",
		/* [29] */
		"JIMMY STEWART",
		/* [30] */
		"JONNY QUEST AND HADJI",
		/* [31] */
		"JOHN 'THE DUKE' WAYNE",
		/* [32] */
		"JODI, BUFFY, AND MR. FRENCH",
		/* [33] */
		"LITTLE ORPHAN ANNIE",
		/* [34] */
		"LONELY HEARTS CLUB BAND",
		/* [35] */
		"LOU COSTELLO",
		/* [36] */
		"LON CHANEY JR.",
		/* [37] */
		"MARY TYLER MOORE",
		/* [38] */
		"MARY, QUEEN OF SCOTS",
		/* [39] */
		"MEL BLANC",
		/* [40] */
		"MR. GREEN JEANS",
		/* [41] */
		"MR. WIZARD",
		/* [42] */
		"MRS. BUTTERWORTH",
		/* [43] */
		"MAN OF A THOUSAND FACES",
		/* [44] */
		"MIGHTY MOUSE",
		/* [45] */
		"NORMAN ROCKWELL",
		/* [46] */
		"NITTY GRITTY DIRT BAND",
		/* [47] */
		"NORMAN BATES",
		/* [48] */
		"NEIL PEART",
		/* [49] */
		"NAPOLEON I",
		/* [50] */
		"ORSON WELLES",
		/* [51] */
		"ONE BULLET BARNEY",
		/* [52] */
		"PRINCE ALBERT",
		/* [53] */
		"PABLO PICASSO",
		/* [54] */
		"REMBRANDT",
		/* [55] */
		"ROBIN LEECH",
		/* [56] */
		"SAMUEL F.B. MORSE",
		/* [57] */
		"SIR WALTER RALEIGH",
		/* [58] */
		"SITTING BULL",
		/* [59] */
		"SINBAD THE SAILOR",
		/* [60] */
		"STEWART COPELAND",
		/* [61] */
		"TARZAN OF THE APES",
		/* [62] */
		"THE DIAL TONES",
		/* [63] */
		"THE FAR SIDE",
		/* [64] */
		"THE HEADLESS HORSEMAN",
		/* [65] */
		"TASMANIAN DEVIL",
		/* [66] */
		"VINCENT VAN GOGH",
		/* [67] */
		"WALLY AND THE BEAVER",
		/* [68] */
		"XERXES",
		/* [69] */
		"YOSEMITE SAM",
		/* [70] */
		"YVES SAINT LAUREN",
		/* [71] */
		"ZIGGY STARDUST"
	}
};

resource 'PUZZ' (135, "Phrase", purgeable) {
	{	/* array StringArray: 64 elements */
		/* [1] */
		"AAARGH",
		/* [2] */
		"BE KIND - REWIND",
		/* [3] */
		"BUILD A BETTER MOUSETRAP",
		/* [4] */
		"BARKING UP THE WRONG TREE",
		/* [5] */
		"BARK LIKE A DOG",
		/* [6] */
		"CAT ON A HOT TIN ROOF",
		/* [7] */
		"CATCH TWENTY-TWO",
		/* [8] */
		"CITIZEN'S ARREST",
		/* [9] */
		"CLIMBING THE WALLS",
		/* [10] */
		"DON'T PUT YOUR LIPS ON THAT",
		/* [11] */
		"DEAD MEN TELL NO LIES",
		/* [12] */
		"DAMN THE TORPEDOS",
		/* [13] */
		"DON'T BE A MR. BUNGLE",
		/* [14] */
		"EARLY TO BED, EARLY TO RISE",
		/* [15] */
		"ELECTRICITY IS YOUR FRIEND",
		/* [16] */
		"FROM RAGS TO RICHES",
		/* [17] */
		"FROM RICHES TO RAGS",
		/* [18] */
		"GET OUT OF MY HAIR",
		/* [19] */
		"GET OFF MY BACK",
		/* [20] */
		"GOOD GUYS ALWAYS FINISH LAST",
		/* [21] */
		"GROOVY MARSHA",
		/* [22] */
		"HIT THE DIRT!",
		/* [23] */
		"HERE I COME TO SAVE THE DAY!",
		/* [24] */
		"HOT ENOUGH FOR YA?",
		/* [25] */
		"HE'S ON CLOUD NINE",
		/* [26] */
		"I'LL FIX THEIR WAGON",
		/* [27] */
		"IT'S NOT SO MUCH THE TEMPERATURE, BUT TH"
		"E HUMIDITY",
		/* [28] */
		"IT JUST CAME TO ME",
		/* [29] */
		"LET'S GO CAMPING",
		/* [30] */
		"MIND YOUR P'S AND Q'S",
		/* [31] */
		"MORE FOR YOUR MONEY",
		/* [32] */
		"MOVE 'EM OUT",
		/* [33] */
		"MIND YOUR OWN BUSINESS",
		/* [34] */
		"MIND YOUR MANNERS",
		/* [35] */
		"MY GOD, IT'S FULL OF STARS",
		/* [36] */
		"NIP IT, NIP IT IN THE BUD",
		/* [37] */
		"OH YOU'RE A HEDGE",
		/* [38] */
		"ON PINS AND NEEDLES",
		/* [39] */
		"PEACE, MAN",
		/* [40] */
		"PORKCHOPS AND APPLESAUCE",
		/* [41] */
		"QUIT WHILE YOU'RE AHEAD",
		/* [42] */
		"QUOTH THE RAVEN",
		/* [43] */
		"REST IN PEACE",
		/* [44] */
		"SALLY SELLS SEASHELLS BY THE SEASHORE",
		/* [45] */
		"SWEEP IT UNDER THE RUG",
		/* [46] */
		"SKELETONS IN THE CLOSET",
		/* [47] */
		"SO WHAT ARE YOU AFRAID OF?",
		/* [48] */
		"SQUEAL LIKE A PIG",
		/* [49] */
		"STICKY WICKET",
		/* [50] */
		"TAKE MY WIFE, PLEASE",
		/* [51] */
		"THERE GOES THE NEIGHBORHOOD",
		/* [52] */
		"THE CAT'S OUT OF THE BAG",
		/* [53] */
		"TO BE OR NOT TO BE",
		/* [54] */
		"THEY'RE HERE",
		/* [55] */
		"THE HAND IS QUICKER THAN THE EYE",
		/* [56] */
		"UNDER A CLEAR BLUE SKY",
		/* [57] */
		"WASH BEHIND YOUR EARS",
		/* [58] */
		"WATER UNDER THE BRIDGE",
		/* [59] */
		"WALKING ON THIN ICE",
		/* [60] */
		"WHAT'S UP DOC?",
		/* [61] */
		"WORDS TO LIVE BY",
		/* [62] */
		"WANNA SHOOT SOME POOL?",
		/* [63] */
		"WHAT'S YOUR MAJOR?",
		/* [64] */
		"YOU KNOW WHAT THEY SAY"
	}
};

