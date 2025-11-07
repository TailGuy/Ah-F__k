#include "maingame.h"
#include "memory.h"
#include <stddef.h>
#include "raymath.h"
#include "ahfuckmath.h"
#include "string.h"
#include <stdio.h> // For snprintf


/* The WHOLE game is dumped in this 1 file, so good luck scrolling and figuring out what's going on. */


// Fields.

/* Idk. */
const float WINDOW_ASPECT_RATIO = 16.0f / 9.0f;

/* Pre start stage. */
const float PRE_START_DURATION_SECONDS = 2.0f;

/* Room animation. */
const float ROOM_ANIMATION_DURATION_SECONDS = 0.5f;


/* Shadows. */
const float SHADOWN_BRIGHTNESS_MAX = 0.1f;
const float SHADOWN_BRIGHTNESS_MIN = 0.0125f;
const uint8_t SHADOW_BRIGHTNESS_NIGHT = (uint8_t)(UINT8_MAX * 0.20f);
const float NIGHT_SHADOW_ENABLE_TIME = 0.5 * 0.15f; // Check the fucking shader for context.

/* Colors. */
const Color COLOR_DAY = (Color){ .r = 157, .g = 198, .b = 225, .a = 255 };
const Color COLOR_EVENING = (Color){ .r = 229, .g = 143, .b = 84, .a = 255 };
const Color COLOR_NIGHT = (Color){ .r = 5, .g = 13, .b = 23, .a = 255 };

/* Shader. */
const int COLOR_STEP_COUNT_MAX = 255;
const int COLOR_STEP_COUNT_MIN = 16;
const float SHADER_RANDOM_UPDATE_FREQUENCY = 10.0f;


/* Paper. */
const Vector2 PAPER_POS_DOWN = (Vector2){ .x = 0.53f, .y = 0.65f };
const Vector2 PAPER_SIZE_DOWN = (Vector2){ .x = 0.5f, .y = 0.5f };
const Vector2 PAPER_SIZE_CHECK = (Vector2){ .x = 0.85f, .y = 0.85f };

const Vector2 DOCUMENT_POS_DOWN = (Vector2){ .x = 0.75f, .y = 0.4f };

const float PAPER_ASPECT_RATIO = 74.0f / 104.0f;

/* Day. */
static const float DAY_DURATION_SECONDS = 10.0f;
static const float SHIFT_DURATION_SECONDS = 20.0f;

/* Camera. */
static const float REQUIRED_CAMERA_OFFSET = 0.05f;
static const float CHECK_PAPER_BOUNDS = 0.15f;


// Static functions.
static float GetRandomFloat()
{  
    // The worst way to do this.
    int MaxValue = 10000;
    return (float)GetRandomValue(0, MaxValue) / (float)MaxValue;
}

static void GenRandomIndexArray(size_t* array, size_t size)
{
    if (size <= 0)
    {
        return;
    }

    for (size_t i = 0; i < size; i++)
    {
        array[i] = i;
    }

    for (size_t i = 0; i < size / 2; i++)
    {
        size_t IndexA = GetRandomValue(0, size - 1);
        size_t IndexB = GetRandomValue(0, size - 1);
        size_t ValueA = array[IndexA];
        size_t ValueB = array[IndexB];
        array[IndexA] = ValueB;
        array[IndexB] = ValueA;
    }
}

static void CopyDocumentsRandomlyIntoSource(Document* source, DocumentSource* destination, size_t documentCount)
{
    size_t IndexArray[MAX_DOCUMENTS];
    GenRandomIndexArray(IndexArray, documentCount);
    for (size_t i = 0; i < documentCount; i++)
    {
        destination->Documents[i] = source[IndexArray[i]]; // Very fucking slow considering the document struct is HUGE.
    }
    destination->Count = documentCount;
}

static void AddTempDocument(Document* documents, size_t* documentCount, DocumentType type, const char* text, bool isCorrect)
{
    if (*documentCount >= MAX_DOCUMENTS)
    {
        return;
    }

    Document* Target = &documents[*documentCount];

    strncpy(Target->Text, text, MAX_DOCUMENT_TEXT_LENGTH);
    Target->Type = type;
    Target->IsCorrect = isCorrect;
    Target->RotationDeg = 0.0f;
    Target->Offset = (Vector2){ .x = 0.0f, .y = 0.0f };

    *documentCount += 1;
}

static void InitDocuments(MainGameContext* self)
{
    Document* Documents = MemAlloc(sizeof(Document) * MAX_DOCUMENTS);
    size_t DocumentCount = 0;

    // NOTE: 'isCorrect' is now the last parameter.
    // true = The document is valid OR it's junk mail that should be trashed.
    // false = The document has an error and should be returned.
    
    // --- Reference & Rulebook Docs --- (Correct Action: Trash, so IsCorrect = true)
    AddTempDocument(Documents, &DocumentCount, DocumentType_LegitButNotForBench, "List of Official Departments:\n\nBench-Related Departments:\nDepartment of Parks & Recreation (ID: #77B)\nMunicipal Budget & Finance Office (ID: #12D)\nPublic Safety & Standards Bureau (ID: #31A)\nOther Legitimate Departments:\nDepartment of Pothole Mitigation (ID: #44G)\nSanitation & Waste Management (ID: #88C)\nPublic Library Services (ID: #65F)\nLamplight & Illumination Committee (ID: #59L)", true);
    AddTempDocument(Documents, &DocumentCount, DocumentType_LegitButNotForBench, "OFFICE OF PROCEDURAL AFFAIRS - RULEBOOK\n\nPAGE 1: DATES\n\nAll official forms, proposals, and requests MUST be dated\nwith the current date.\n\nFuture-dated and past-dated documents are to be considered\ninvalid and must be rejected immediately.\n\nThe official date format is MM/DD/YY. Any other format\nwill be deemed non-compliant.\n\nSIGNATURE: _________________________", true);
    AddTempDocument(Documents, &DocumentCount, DocumentType_LegitButNotForBench, "OFFICE OF PROCEDURAL AFFAIRS - RULEBOOK\n\nPAGE 2: DEPARTMENTS\n\nAll correspondence must originate from or be directed to an\nofficially sanctioned department with the correct ID.\n\nBench-Related Departments:\n- Parks & Recreation.........................ID: #77B\n- Budget & Finance...........................ID: #12D\n- Public Safety & Standards..................ID: #31A\n\nOther Sanctioned Departments:\n- Pothole Mitigation.........................ID: #44G\n- Sanitation & Waste.........................ID: #88C\n- Public Library Services....................ID: #65F\n- Lamplight & Illumination...................ID: #59L\n\nSubmissions from unsanctioned departments are invalid.\nSIGNATURE: _________________________", true);
    AddTempDocument(Documents, &DocumentCount, DocumentType_LegitButNotForBench, "OFFICE OF PROCEDURAL AFFAIRS - RULEBOOK\n\nPAGE 3: BUDGETS\n\nAll project proposals must adhere to the standard budget\nguidelines.\n\n- Projects with a total cost of up to $500.00 are\n  eligible for Standard Approval.\n\n- Projects with a total cost exceeding $500.00 require\n  an additional Form H-77 \"Excessive Expenditure Review\"\n  and must be rejected if this form is not attached.\n\nAll costs must be itemized.\n\nSIGNATURE: _________________________", true);
    AddTempDocument(Documents, &DocumentCount, DocumentType_LegitButNotForBench, "OFFICE OF PROCEDURAL AFFAIRS - RULEBOOK\n\nPAGE 4: MATERIALS\n\nThe Public Safety & Standards Bureau has mandated a list of\napproved and prohibited materials for park assets.\n\nApproved Materials:\n- Steel (Galvanized)\n- Aluminum (Powder-Coated)\n- Concrete (Reinforced)\n- Recycled Composite Plastic\n\nProhibited Materials:\n- Wood (All types - Splinter risk)\n- Uncoated Iron (Rust risk)\n- Glass (Breakage risk)\n- Plush Velvet (Sanitation risk)\n\nProposals with prohibited materials must be rejected.\nSIGNATURE: _________________________", true);
    
    // --- Bench-Related Docs ---
    AddTempDocument(Documents, &DocumentCount, DocumentType_LegitForBench, "PROJECT PROPOSAL: NEW PARK ASSET\nDATE: 10/27/25\nDEPT: Department of Parks & Recreation (ID: #77B)\n\nThis document serves as a formal request to install one (1)\nnew public seating bench in Municipal Park, Sector 4.\n\nThe proposed location has high foot traffic and currently\nlacks adequate seating for our senior citizens.\n\nProposed Material: Steel (Galvanized)\nEstimated Budget: $450.00\n\nThis project is considered essential for public comfort.\nWe await your swift approval to proceed.\n\nSIGNATURE: _________________________", true);
    AddTempDocument(Documents, &DocumentCount, DocumentType_LegitForBench, "PROJECT PROPOSAL: NEW PARK ASSET\nDATE: 10/28/25\nDEPT: Department of Parks & Recreation (ID: #77B)\n\nThis document is a formal request for a new seating unit\nin Municipal Park, Sector 2, near the duck pond.\n\nThe previous bench was removed due to rust damage.\n\nProposed Material: Steel (Galvanized)\nEstimated Budget: $499.99\n\nWe believe this falls under the standard approval process.\nThank you for your timely consideration.\n\nSIGNATURE: _________________________", false); // Incorrect: Future Date
    AddTempDocument(Documents, &DocumentCount, DocumentType_LegitForBench, "PROJECT PROPOSAL: NEW PARK ASSET\nDATE: 10/27/25\nDEPT: Department of Parks & Recreation (ID: #77C)\n\n[ERROR: Incorrect Department ID]\n\nRequest to install a bench in Municipal Park, Sector 1.\nThe community has been asking for this for months.\n\nWe need to get this done before the winter holidays.\n\nProposed Material: Steel (Galvanized)\nEstimated Budget: $420.00\n\nSIGNATURE: _________________________", false); // Incorrect: Department ID
    AddTempDocument(Documents, &DocumentCount, DocumentType_LegitForBench, "PROJECT PROPOSAL: NEW PARK ASSET\nDATE: 10/26/25\nDEPT: Department of Parks & Recreation (ID: #77B)\n\n[ERROR: Date is in the past]\n\nWe are requesting the immediate installation of one (1)\npublic bench. Location is the main entrance of the park.\n\nProposed Material: Recycled Composite Plastic\nEstimated Budget: $380.00\n\nThis is a high-priority request.\n\nSIGNATURE: _________________________", false); // Incorrect: Past Date
    AddTempDocument(Documents, &DocumentCount, DocumentType_LegitForBench, "PROJECT PROPOSAL: NEW PARK ASSET\nDATE: 10/27/25\nDEPT: Department of Parks & Recreation (ID: #77B)\n\n[ERROR: Budget exceeds $500]\n\nFormal request for a \"deluxe\" model public seating bench\nfor the mayor's garden section of Municipal Park.\n\nThis model includes extra armrests and lumbar support.\n\nProposed Material: Aluminum (Powder-Coated)\nEstimated Budget: $650.00\n\nSIGNATURE: _________________________", false); // Incorrect: Budget
    AddTempDocument(Documents, &DocumentCount, DocumentType_LegitForBench, "PROJECT PROPOSAL: NEW PARK ASSET\nDATE: 10/27/25\nDEPT: Department of Parks & Recreation (ID: #77B)\n\n[ERROR: Prohibited Material]\n\nThis is a request to install a beautiful, rustic-style\nbench near the old oak tree in Municipal Park.\n\nWe feel a natural aesthetic is best for this location.\n\nProposed Material: Wood (Oak, Varnished)\nEstimated Budget: $490.00\n\nSIGNATURE: _________________________", false); // Incorrect: Material
    AddTempDocument(Documents, &DocumentCount, DocumentType_LegitForBench, "BUDGETARY REVIEW: F-37\nDATE: 10/27/25\nDEPT: Municipal Budget & Finance Office (ID: #12D)\n\nRegarding the proposal for a new bench in Municipal Park:\n\nThe submitted budget of $450.00 has been reviewed by our\noffice and found to be within acceptable parameters for\na Standard Approval project.\n\nFunds have been earmarked pending Final Approval.\n\nNo further action from this department is required unless\nthe total cost changes.\n\nSIGNATURE: _________________________", true);
    AddTempDocument(Documents, &DocumentCount, DocumentType_LegitForBench, "BUDGETARY REVIEW: F-38\nDATE: 10/27/25\nDEPT: Municipal Budget & Finance Office (ID: #12D)\n\n[ERROR: Budget exceeds $500, requires H-77]\n\nThe submitted budget of $650.00 for the \"deluxe\" bench\nhas been reviewed. This cost exceeds the standard limit.\n\nAs per Rulebook Page 3, this requires Form H-77.\nThe proposal is rejected pending submission of the\ncorrect documentation.\n\nSIGNATURE: _________________________", true); // Correctly filed rejection
    AddTempDocument(Documents, &DocumentCount, DocumentType_LegitForBench, "SAFETY COMPLIANCE REVIEW: F-37\nDATE: 10/27/25\nDEPT: Public Safety & Standards Bureau (ID: #31A)\n\nThe proposal for a Galvanized Steel bench has been reviewed\nby this bureau.\n\nSteel is an approved material. Provided the installation\nis performed by a certified technician and there are no\nsharp edges, this project meets safety standards.\n\nOur department grants its preliminary approval.\n\nSIGNATURE: _________________________", true);
    AddTempDocument(Documents, &DocumentCount, DocumentType_LegitForBench, "SAFETY COMPLIANCE REVIEW: F-39\nDATE: 10/27/25\nDEPT: Public Safety & Standards Bureau (ID: #31A)\n\n[ERROR: Prohibited Material]\n\nThe proposal for a Varnished Oak bench is rejected.\n\nAs per Rulebook Page 4, all types of wood are strictly\nprohibited for new public assets due to the high risk\nof splinters and degradation.\n\nPlease resubmit with an approved material.\n\nSIGNATURE: _________________________", true); // Correctly filed rejection
    AddTempDocument(Documents, &DocumentCount, DocumentType_LegitForBench, "CLARIFICATION REQUEST\nDATE: 10/27/25\nDEPT: Department of Parks & Recreation (ID: #77B)\n\nRegarding your rejection of our proposal for a wooden\nbench... are you absolutely sure? We have a lot of surplus\nwood from the storm last year we were hoping to use.\n\nIt would save the city money. Please advise.\n\nCan we get an exemption?\n\nSIGNATURE: _________________________", true);
    AddTempDocument(Documents, &DocumentCount, DocumentType_LegitForBench, "PROJECT AMENDMENT: F-37\nDATE: 10/27/25\nDEPT: Department of Parks & Recreation (ID: #77B)\n\nPer feedback from your office, we are amending our\noriginal proposal.\n\nThe new proposed material is Concrete (Reinforced).\nThis increases the budget slightly.\n\nNew Estimated Budget: $495.00\n\nPlease review this updated proposal.\n\nSIGNATURE: _________________________", true);
    AddTempDocument(Documents, &DocumentCount, DocumentType_LegitForBench, "BUDGETARY REVIEW: F-37 AMENDED\nDATE: 10/27/25\nDEPT: Municipal Budget & Finance Office (ID: #12D)\n\nWe have reviewed the amended budget of $495.00 for the\nconcrete bench.\n\nThis amount is still within the standard approval limit.\nThe allocated funds have been adjusted.\n\nNo further action is needed from our department.\n\nSIGNATURE: _________________________", true);
    AddTempDocument(Documents, &DocumentCount, DocumentType_LegitForBench, "SAFETY COMPLIANCE REVIEW: F-37 AMENDED\nDATE: 10/27/25\nDEPT: Public Safety & Standards Bureau (ID: #31B)\n\n[ERROR: Incorrect Department ID]\n\nThe proposal for a reinforced concrete bench is approved\nby this bureau. Concrete is a durable and safe material.\n\nOur approval is contingent on a smooth, non-abrasive\nfinish on all seating surfaces.\n\nSIGNATURE: _________________________", false); // Incorrect: Department ID
    AddTempDocument(Documents, &DocumentCount, DocumentType_LegitForBench, "FINAL APPROVAL FORM\nDATE: 10/27/25\nDEPT: Department of Parks & Recreation (ID: #77B)\n\nProject F-37: New Bench, Municipal Park.\n\nPlease apply the final approval stamp if all supporting\ndocuments from other departments are in order.\n\nParks & Rec Stamp: [APPROVED]\nFinance Office Stamp: [APPROVED]\nSafety Bureau Stamp: [PENDING]\n\nWe are ready to proceed as soon as we get the green light.\n\nSIGNATURE: _________________________", true);
    
    // --- Other Legit Docs & Ads --- (Correct Action: Trash, so IsCorrect = true)
    AddTempDocument(Documents, &DocumentCount, DocumentType_LegitButNotForBench, "WORK ORDER: POTHOLE REPAIR\nDATE: 10/27/25\nDEPT: Department of Pothole Mitigation (ID: #44G)\n\nA large pothole has been reported on the corner of 5th\nand Main Street. Vehicle damage has been reported.\n\nRequesting a crew to be dispatched with 2 tons of asphalt\nfor immediate repair.\n\nThis is a public safety hazard and should be treated as\na high-priority task.\n\nSIGNATURE: _________________________", true);
    AddTempDocument(Documents, &DocumentCount, DocumentType_LegitButNotForBench, "FUNDING REQUEST: LIBRARY BOOKS\nDATE: 10/27/25\nDEPT: Public Library Services (ID: #65F)\n\nThe Municipal Library's budget for new acquisitions for\nthis fiscal quarter has been exhausted.\n\nWe are requesting an emergency allocation of $450.00 for\nthe purchase of new children's literature.\n\nLiteracy is a cornerstone of our community.\n\nSIGNATURE: _________________________", true);
    AddTempDocument(Documents, &DocumentCount, DocumentType_LegitButNotForBench, "MAINTENANCE TICKET: STREETLIGHT OUTAGE\nDATE: 10/27/25\nDEPT: Lamplight & Illumination Committee (ID: #59L)\n\nStreetlight #S-1138 on Elm Street is non-functional.\nThe area is dangerously dark at night.\n\nRequesting a certified electrician to be dispatched to\nreplace the bulb and ballast.\n\nPlease expedite this repair.\n\nSIGNATURE: _________________________", true);
    AddTempDocument(Documents, &DocumentCount, DocumentType_LegitButNotForBench, "SUPPLY ORDER: RECYCLING BINS\nDATE: 10/27/25\nDEPT: Sanitation & Waste Management (ID: #88C)\n\nWe are requesting the purchase of 250 new blue recycling\nbins for the upcoming city-wide recycling initiative.\n\nBudget: $4,500.00 (Pre-approved under green initiative)\n\nThis order is critical to the program's success.\n\nSIGNATURE: _________________________", true);
    AddTempDocument(Documents, &DocumentCount, DocumentType_LegitButNotForBench, "WORK ORDER: POTHOLE REPAIR\nDATE: 10/25/25\nDEPT: Department of Pothole Mitigation (ID: #44G)\n\n[ERROR: Date is in the past]\n\nRequest to fill a series of small potholes along the\nentirety of Park Avenue.\n\nThe road surface is becoming a washboard. We need at\nleast 4 tons of asphalt and a full day crew.\n\nSIGNATURE: _________________________", true);
    AddTempDocument(Documents, &DocumentCount, DocumentType_LegitButNotForBench, "FUNDING REQUEST: E-BOOK LICENSES\nDATE: 10/27/25\nDEPT: Public Library Services (ID: #65F)\n\nOur digital lending licenses for several popular authors\nare set to expire next month.\n\nWe request $750.00 to renew these licenses to ensure\nuninterrupted service for our patrons.\n\nDigital access is a key service we provide.\n\nSIGNATURE: _________________________", true);
    AddTempDocument(Documents, &DocumentCount, DocumentType_LegitButNotForBench, "MAINTENANCE TICKET: FAULTY WIRING\nDATE: 10/27/25\nDEPT: Lamplight & Illumination Committee (ID: #55L)\n\n[ERROR: Incorrect Department ID]\n\nThe lamppost at the entrance to City Hall is flickering\nerratically. We suspect faulty wiring.\n\nThis is not only an illumination issue but a potential\nfire hazard. Urgent action is required.\n\nSIGNATURE: _________________________", true);
    AddTempDocument(Documents, &DocumentCount, DocumentType_LegitButNotForBench, "WORK ORDER: GRAFFITI REMOVAL\nDATE: 10/27/25\nDEPT: Sanitation & Waste Management (ID: #88C)\n\nGraffiti has been reported on the western wall of the\nmunicipal swimming pool.\n\nRequesting a sanitation crew with a power washer and\nchemical solvents to be dispatched for removal.\n\nMaintaining the appearance of our public facilities is\nimportant for community morale.\n\nSIGNATURE: _________________________", true);
    AddTempDocument(Documents, &DocumentCount, DocumentType_LegitButNotForBench, "MEETING MINUTES\nDATE: 10/26/25\nDEPT: Lamplight & Illumination Committee (ID: #59L)\n\nThe weekly meeting was held.\nAgenda:\n1. Discussed ongoing bulb replacement schedule.\n2. Reviewed budget for decorative holiday lighting.\n3. Debated the merits of LED vs Sodium-vapor lamps.\n\nMeeting adjourned. Further action pending budget review.\n\nSIGNATURE: _________________________", true);
    AddTempDocument(Documents, &DocumentCount, DocumentType_LegitButNotForBench, "INVOICE\nDATE: 10/27/25\nDEPT: Department of Pothole Mitigation (ID: #44G)\n\nTo: Municipal Budget & Finance Office\n\nInvoice for 15 tons of \"Premium Asphalt Mix\" from\n\"Steve's Pothole Emporium\".\n\nTotal: $4,985.32\n\nPlease remit payment within 30 days. Thank you.\n\nSIGNATURE: _________________________", true);
    AddTempDocument(Documents, &DocumentCount, DocumentType_Advertisement, "** IS YOUR STAPLER FEELING SAD? **\n\nDon't settle for a gloomy, jam-prone stapler.\nUpgrade to the new SWINGLINE 9000-S!\n\nWith patented Jam-Free Technology(TM) and a sleek,\naerodynamic design, it's the last stapler you'll\never need.\n\nComes in Municipal Grey, Bureaucratic Beige, and\nRage-Red.\n\n** CALL NOW! OPERATORS ARE STANDING BY! **", true);
    AddTempDocument(Documents, &DocumentCount, DocumentType_Advertisement, ">>> TIRED OF ALL THIS PAPER? <<<\n\nGo digital with FormFlow Pro X! Our revolutionary\nsoftware will turn your mountain of paper into a...\nslightly more organized digital mountain of paper!\n\nIt's the future! Probably!\n\n*Subscription required. Mouse and keyboard not\nincluded. May not actually reduce workload. We are\nnot liable for digital papercuts.*", true);
    AddTempDocument(Documents, &DocumentCount, DocumentType_Advertisement, "** GLORIA'S CAFETERIA **\n(Located in the basement)\n\nTODAY'S SPECIAL: THE BUREAUCRAT'S BURDEN BOWL\n\nA lukewarm serving of grey stew, served with a single,\nslightly stale cracker. It's food.\n\nJust like your job, it will sustain you, but you\nwon't enjoy it.\n\nNow with 10% less existential dread! (Not guaranteed)", true);
    AddTempDocument(Documents, &DocumentCount, DocumentType_Advertisement, "$$$ ARE YOU A WINNER? YOU COULD BE! $$$\n\nYou, yes YOU, may have been selected for a once-in-a\n-lifetime opportunity to join our Multi-Level Stamping\ndownline!\n\nSell high-quality, artisanal ink pads to your friends,\nfamily, and colleagues. Be your own boss!\n\nJust attend our 3-hour \"informational seminar\" to\nlearn more. (Attendance fee required).", true);
    AddTempDocument(Documents, &DocumentCount, DocumentType_Advertisement, "** \"THE INFINITE LOOP\" **\n\nA new motivational poster!\n\nFeaturing a picture of a hamster on a wheel, this\nposter will remind you to always keep pushing, even if\nyou're not entirely sure where you're going.\n\nPerfect for livening up any drab cubicle wall.\n\n\"Keep running, the cheese is just a metaphor!\"", true);
    AddTempDocument(Documents, &DocumentCount, DocumentType_Advertisement, "INTER-OFFICE MEMO: MANDATORY FUN COMMITTEE\n\nDon't forget this Friday is \"Hawaiian Shirt Friday\"!\n\nLet's bring a taste of the tropics to the drudgery of\nour daily lives. Participation is not optional.\n\nPrizes will be awarded for the most aggressively\nfestive shirt.\n\nLet's get tropical!\n\n- The M.F.C.", true);
    AddTempDocument(Documents, &DocumentCount, DocumentType_Advertisement, "LOST & FOUND: ONE (1) WILL TO LIVE\n\nHas anyone seen a misplaced will to live?\n\nLast seen sometime around Monday morning near the\ncoffee machine. Not particularly vibrant or strong,\nbut answers to the name \"Hope\".\n\nIf found, please return to cubicle 7-C.\n\nNo reward, but I will offer a sigh of weary gratitude.", true);
    AddTempDocument(Documents, &DocumentCount, DocumentType_Advertisement, "YOUR SPINE HATES YOU. WE CAN HELP.\n\nThe Ergo-Chair 500. It's a chair. It's slightly more\ncomfortable than the rock-hard slab of plastic you're\ncurrently sitting on.\n\nFeatures:\n- It goes up.\n- It goes down.\n- Wheels that only squeak moderately.\n\nInvest in your posture. Your future self will thank\nyou with slightly fewer back problems.", true);
    AddTempDocument(Documents, &DocumentCount, DocumentType_Advertisement, "** COFFEE COFFEE COFFEE **\n\nIs your blood just blood? Fix that.\nOur new \"Midnight Oil\" blend is so strong, you'll be\nable to see through time.\n\nSide effects may include: jittering, paranoia, the\nability to hear colors, and extreme productivity.\n\nAvailable in the breakroom for a nominal fee.", true);
    AddTempDocument(Documents, &DocumentCount, DocumentType_Advertisement, "CLASSIFIED AD\n\nFOR TRADE: One (1) slightly used red stapler.\nHas sentimental value.\n\nWANTED: A single shred of recognition from my\nsuperiors. Acknowledgment of my existence. A brief,\nfleeting moment of professional satisfaction.\n\nWill also accept a donut.\n\nContact Milton, basement level.", true);
    CopyDocumentsRandomlyIntoSource(Documents, &self->AdsDocsSource, DocumentCount);

    DocumentCount = 0;
    for (size_t i = 0; i < MAX_DOCUMENTS; i++)
    {
        AddTempDocument(Documents, &DocumentCount, DocumentType_Blank, "", true);
    }

    MemFree(Documents);
}


void BreakTextIntoLinesInPlace(char* text, size_t maxLineLength)
{
    size_t LineStart = 0;
    size_t LastSpace = 0;
    size_t Index = 0;

    while(text[Index])
    {
        if (text[Index] == ' ')
        {
            LastSpace = Index;
        }
        else if (text[Index] == '\n')
        {
            LineStart = Index + 1;
            Index++;
            continue;
        }

        else if(Index - LineStart >= maxLineLength && LastSpace > LineStart)
        {
            text[LastSpace] = '\n';
            LineStart = LastSpace + 1;
        }

        Index++;
    }
}

static void AddDocument(MainGameContext* self, Document* source)
{
    if (self->DocumentCount >= MAX_DOCUMENTS)
    {
        return;
    }

    Document* TargetDocument = &self->Documents[self->DocumentCount];

    TargetDocument->Type = source->Type;
    TargetDocument->IsCorrect = source->IsCorrect;
    strncpy(TargetDocument->Text, source->Text, MAX_DOCUMENT_TEXT_LENGTH);
    int MAX_CHARS_PER_LINE = 60 * PAPER_ASPECT_RATIO;
    BreakTextIntoLinesInPlace(TargetDocument->Text, MAX_CHARS_PER_LINE);

    const float MAX_ROTATION_DEG = 3.0f;
    const float MAX_OFFSET = 0.005f;

    TargetDocument->RotationDeg = ((GetRandomFloat() - 0.5f )* 2.0f) * MAX_ROTATION_DEG;
    TargetDocument->Offset = (Vector2)
    {
        .x = ((GetRandomFloat() - 0.5f )* 2.0f) * MAX_OFFSET,
        .y = ((GetRandomFloat() - 0.5f )* 2.0f) * MAX_OFFSET
    };

    self->DocumentCount++;
}

/* States. */
static void PreStartUpdate(MainGameContext* self, AssetCollection* assets, float deltaTime, AhFuckRenderer* renderer, AudioContext* audio)
{
    self->PreStartState.ElapsedStateDuration += deltaTime;
    if (self->PreStartState.ElapsedStateDuration >= PRE_START_DURATION_SECONDS)
    {
        self->State = GameState_InGame;
        renderer->GlobalScreenOpacity = 1.0f;
        Audio_PlaySound(audio, assets->BackgroundMusic, true, 0.4f);
    }
    else
    {
        renderer->GlobalScreenOpacity = self->PreStartState.ElapsedStateDuration / PRE_START_DURATION_SECONDS;
    }

    UNUSED(renderer);
}

static void OnTrashPaper(MainGameContext* self, AssetCollection* assets, AhFuckRenderer* renderer, AudioContext* audio)
{
    if (!self->IsPaperOnTable || !self->ActiveDocument)
    {
        return;
    }

    if (self->ActiveDocument->Type == DocumentType_Advertisement || self->ActiveDocument->Type == DocumentType_LegitButNotForBench)
    {
        self->Score++;
    }
    else
    {
        self->Score--;
    }

    UNUSED(renderer);
    self->ActiveDocument = NULL;
    self->IsPaperOnTable = false;
    Audio_PlaySound(audio, assets->TrashSound, false, 0.7f);
}

static void OnSubmitPaper(MainGameContext* self, AssetCollection* assets, AhFuckRenderer* renderer, AudioContext* audio)
{
    if (!self->IsPaperOnTable || !self->ActiveDocument) return;

    if (self->ActiveDocument->Type == DocumentType_LegitForBench && self->ActiveDocument->IsCorrect)
    {
        self->Score++;
    }
    else
    {
        self->Score--;
    }

    self->ActiveDocument = NULL;
    self->IsPaperOnTable = false;
    Audio_PlaySound(audio, assets->PaperSound, false, 0.5f);
    UNUSED(renderer);
}

static void OnReturnPaper(MainGameContext* self, AssetCollection* assets, AhFuckRenderer* renderer, AudioContext* audio)
{
    if (!self->IsPaperOnTable || !self->ActiveDocument) return;

    // Returning a bench document that has an error is the correct action.
    if (self->ActiveDocument->Type == DocumentType_LegitForBench && !self->ActiveDocument->IsCorrect)
    {
        self->Score++;
    }
    else
    {
        self->Score--;
    }

    self->ActiveDocument = NULL;
    self->IsPaperOnTable = false;
    Audio_PlaySound(audio, assets->PaperSound, false, 0.5f);
    UNUSED(renderer);
}

static bool IsInPaperCheckActionBounds(AhFuckRenderer* renderer)
{
    float MouseY = renderer->MousePosition.y;
    return (MouseY >= (1.0f - CHECK_PAPER_BOUNDS)) || (MouseY <= CHECK_PAPER_BOUNDS);
}

static void OnCheckPaper(MainGameContext* self, AssetCollection* assets, AhFuckRenderer* renderer, AudioContext* audio)
{
    UNUSED(renderer);
    if (!self->IsPaperOnTable)
    {
        return;
    }

    Audio_PlaySound(audio, assets->PaperSound, false, 0.25f);

    self->IsCheckingPaper = true;
}

static void OnStopCheckPaper(MainGameContext* self, AssetCollection* assets, AudioContext* audio)
{
    Audio_PlaySound(audio, assets->PaperSound, false, 0.25f);
    self->IsCheckingPaper = false;
    self->IsCameraMovementAllowed = false;
}

static void UpdatePaperCheckState(MainGameContext* self, float deltaTime, AssetCollection* assets, AhFuckRenderer* renderer, AudioContext* audio)
{
    UNUSED(assets);
    UNUSED(renderer);
    UNUSED(audio);

    float Step = self->IsCheckingPaper ? 1.0f : -1.0f;
    float CHANGE_DURATION = 0.15f;
    self->CheckPaperState = Clamp(self->CheckPaperState + (deltaTime / CHANGE_DURATION) * Step, 0.0f, 1.0f);
}

static void EnsureAnimationControls(MainGameContext* self, AhFuckRenderer* renderer)
{
    if (!IsInPaperCheckActionBounds(renderer))
    {
        self->IsCameraMovementAllowed = true;
    }

    if (self->RoomAnimationDirection || self->IsCheckingPaper || !self->IsCameraMovementAllowed)
    {
        return;
    }

    if (renderer->MousePosition.y <= REQUIRED_CAMERA_OFFSET)
    {
        self->RoomAnimationDirection = 1;        
    }
    else if (renderer->MousePosition.y >= (1.0f - REQUIRED_CAMERA_OFFSET))
    {
        self->RoomAnimationDirection = -1;        
    }
}

static inline bool IsNearDesk(MainGameContext* self)
{
    return self->AnimationIndex >= (ROOM_ANIMATION_FRAME_COUNT - 1);
}

static Rectangle GetItemBounds(AhFuckRenderer* renderer, Vector2 pos, Vector2 size, float textureAspectRatio)
{
    Vector2 Size = (Vector2){ 1.0 * textureAspectRatio, .y = 1.0 };
    Size = Vector2Multiply(Size, size);
    Size = Renderer_AdjustVector(renderer, Size);
    Vector2 HalfSize = Vector2Divide(Size, (Vector2){ .x = 2.0f, .y = 2.0f });
    Vector2 Min = Vector2Subtract(pos, HalfSize);
    return (Rectangle){ .x = Min.x, .y = Min.y, .width = Size.x, .height = Size.y };
}

static Rectangle GetPaperBounds(MainGameContext* self, AhFuckRenderer* renderer)
{
    return GetItemBounds(renderer, self->PaperPosition, PAPER_SIZE_DOWN, PAPER_ASPECT_RATIO);
}

static Rectangle GetDocumentStackBounds(MainGameContext* self, AhFuckRenderer* renderer)
{
    UNUSED(self);
    UNUSED(renderer);
    
    // Define a simple, generous rectangle around the document stack's visual position.
    // The stack is centered around DOCUMENT_POS_DOWN = {0.75f, 0.4f}.
    // A box of 0.4 width and 0.6 height should be more than enough to cover it.
    const float grabWidth = 0.4f;
    const float grabHeight = 0.6f;
    
    return (Rectangle){ 
        .x = DOCUMENT_POS_DOWN.x - (grabWidth / 2.0f), 
        .y = DOCUMENT_POS_DOWN.y - (grabHeight / 2.0f),
        .width = grabWidth, 
        .height = grabHeight 
    };
}

static Rectangle GetTrashBounds(MainGameContext* self, AhFuckRenderer* renderer)
{
    UNUSED(self);
    UNUSED(renderer);
    float BoundsSize = 0.23f;
    return (Rectangle){ .x = 0.0f, .y = 0.0f, .width = BoundsSize, .height = 1.0f };
}

static Rectangle GetSubmitBounds(MainGameContext* self, AhFuckRenderer* renderer)
{
    UNUSED(self);
    UNUSED(renderer);
    float BoundsSize = 0.23f;
    return (Rectangle){ .x = 1.0f - BoundsSize, .y = 0.0f, .width = BoundsSize, .height = 1.0f };
}

static Rectangle GetReturnBounds(MainGameContext* self, AhFuckRenderer* renderer)
{
    UNUSED(self);
    UNUSED(renderer);
    float BoundsSize = 0.23f;
    // This is a zone at the top of the screen, but not in the corners which are for submit/trash
    return (Rectangle){ .x = BoundsSize, .y = 0.0f, .width = 1.0f - (BoundsSize * 2), .height = BoundsSize };
}


static inline bool IsPosInBounds(Vector2 pos, Rectangle bounds)
{
    return CheckCollisionPointRec(pos, bounds);
}

static void MovePaperTowards(MainGameContext* self, Vector2 pos, float deltaTime, AhFuckRenderer* renderer)
{
    UNUSED(renderer);

    Vector2 PaperTargetPos = pos;
    Vector2 PaperPosToTargetPos = Vector2Subtract(PaperTargetPos, self->PaperPosition);

    const float MOVE_TIME = 0.05f;
    float MoveAmount = 1.0f / MOVE_TIME * deltaTime;
    Vector2 PaperPos = self->PaperPosition;
    PaperPos.x += PaperPosToTargetPos.x * MoveAmount;
    PaperPos.y += PaperPosToTargetPos.y * MoveAmount;

    self->PaperPosition =PaperPos;
}

static void UpdatePaperHeightTowards(MainGameContext* self, float direction, float deltaTime)
{
    float PAPER_HEIGHT_CHANGE_TIME = 0.20f;
    float PaperHeightChange = deltaTime / PAPER_HEIGHT_CHANGE_TIME;
    self->PaperHeight = Clamp(self->PaperHeight + (PaperHeightChange * direction) , 0.0f, 1.0f);
}

static void UpdatePaperData(MainGameContext* self, AssetCollection* assets, AudioContext* audio, float deltaTime, AhFuckRenderer* renderer)
{
    if (!IsNearDesk(self))
    {
        self->PaperPosition = PAPER_POS_DOWN;
        self->IsCheckingPaper = false;
        return;
    }

    
    if (self->IsCheckingPaper)
    {
        if (renderer->MousePosition.y >= (1.0f - CHECK_PAPER_BOUNDS))
        {
            OnStopCheckPaper(self, assets, audio);
        }
        MovePaperTowards(self, PAPER_POS_DOWN, deltaTime, renderer);
        UpdatePaperHeightTowards(self, -1.0f, deltaTime);
        return;
    }

    Rectangle PaperBounds = GetPaperBounds(self, renderer);
    bool IsOverPaper = IsPosInBounds(renderer->MousePosition, PaperBounds);
    bool IsPaperSelected = (IsOverPaper || self->IsPaperSelected) && IsMouseButtonDown(MOUSE_BUTTON_LEFT);

    if (!IsPaperSelected)
    {
        self->IsPaperSelected = false;

        // Check drop zones when mouse is released
        if (self->IsPaperOnTable)
        {
            if (IsPosInBounds(self->PaperPosition, GetTrashBounds(self, renderer)))
            {
                OnTrashPaper(self, assets, renderer, audio);
                return;
            }
            else if (IsPosInBounds(self->PaperPosition, GetSubmitBounds(self, renderer)))
            {
                OnSubmitPaper(self, assets, renderer, audio);
                return;
            }
            else if (IsPosInBounds(self->PaperPosition, GetReturnBounds(self, renderer)))
            {
                OnReturnPaper(self, assets, renderer, audio);
                return;
            }
        }
        
        if (!self->IsCheckingPaper && (self->PaperPosition.y <= CHECK_PAPER_BOUNDS))
        {
            OnCheckPaper(self, assets, renderer, audio);
        }

        MovePaperTowards(self, PAPER_POS_DOWN, deltaTime, renderer);
        UpdatePaperHeightTowards(self, -1.0f, deltaTime);
    }
    else
    {
        MovePaperTowards(self, IsPaperSelected ? renderer->MousePosition : PAPER_POS_DOWN, deltaTime, renderer);
        self->IsPaperSelected = IsPaperSelected;
        UpdatePaperHeightTowards(self, (IsPaperSelected ? 1.0f : -1.0f), deltaTime);
    }
}

static void UpdateGameTtime(MainGameContext* self, float deltaTime)
{
    self->GameTime += deltaTime;

    // This assumes that shift duration >= day duration (like everything else in this code).
    float DurationWithoutDayAdvance = (SHIFT_DURATION_SECONDS - DAY_DURATION_SECONDS);
    float DurationWithoutDayAdvancePerPart = DurationWithoutDayAdvance / 2.0f;

    if (self->GameTime <= DurationWithoutDayAdvancePerPart)
    {
        self->DayTime = 1.0f;
    }
    else if (self->GameTime >= (SHIFT_DURATION_SECONDS - DurationWithoutDayAdvancePerPart))
    {
        self->DayTime = 0.0f;
    }
    else
    {
        self->DayTime = (SHIFT_DURATION_SECONDS - self->GameTime - DurationWithoutDayAdvancePerPart) / DAY_DURATION_SECONDS;
    }
}

static void UpdateDocumentStack(MainGameContext* self, float deltaTime, AhFuckRenderer* renderer, AssetCollection* assets, AudioContext* audio)
{
    UNUSED(deltaTime);

    if ((self->DocumentCount <= 0)
        || !IsNearDesk(self)
        || !IsPosInBounds(renderer->MousePosition, GetDocumentStackBounds(self, renderer))
        || self->IsPaperOnTable)
    {
        return;
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        self->IsPaperOnTable = true;
        self->PaperPosition = renderer->MousePosition;
        self->ActiveDocument = &self->Documents[self->DocumentCount - 1];
        self->DocumentCount--;
        Audio_PlaySound(audio, assets->PaperSound, false, 0.5f);
    }
}

static void InGameUpdate(MainGameContext* self,
    AssetCollection* assets,
    AudioContext* audio,
    float deltaTime,
    AhFuckRenderer* renderer)
{
    float currentStress = 0.0f;
    if (self->DayDocumentStartCount > 0)
    {
        // Calculate stress as a ratio of remaining documents to the starting amount.
        currentStress = (float)self->DocumentCount / (float)self->DayDocumentStartCount;
    }

    // SanityFactor is the inverse of stress (1.0 = sane, 0.0 = insane)
    self->SanityFactor = 1.0f - currentStress;

    // The audio distortion is directly linked to the stress level.
    audio->AudioFuckShitUpAmount = currentStress;

    EnsureAnimationControls(self, renderer);
    UpdateDocumentStack(self, deltaTime, renderer, assets, audio);
    UpdatePaperData(self, assets, audio, deltaTime, renderer);
    UpdateGameTtime(self, deltaTime);
    UpdatePaperCheckState(self, deltaTime, assets, renderer, audio);

    UNUSED(deltaTime);
}

/* Rendering. */
static void DrawIndicators(MainGameContext* self, AssetCollection* assets, AhFuckRenderer* renderer, float deltaTime)
{
    UNUSED(self);
    UNUSED(deltaTime);
    const float ANIMATION_DURATION = 0.2f;

    Vector2 Margin = (Vector2){.x = 0.5f, .y = 0.4f };
    Margin.x /= renderer->AspectRatio;

    bool IsAtDesk = IsNearDesk(self) && self->IsPaperSelected;
    Vector2 MousePos = renderer->MousePosition;
    float StepTrash = (MousePos.x <= Margin.x && IsAtDesk) ? 1.0f : -1.0f;
    float StepReturn = (MousePos.y <= Margin.y && IsAtDesk) ? 1.0f : -1.0f;
    float StepSubmit = (MousePos.x >= 1.0f - Margin.x && IsAtDesk) ? 1.0f : -1.0f;

    float ChangeGlobal = deltaTime / ANIMATION_DURATION;
    self->TrashIndicatorValue = Clamp(self->TrashIndicatorValue + (ChangeGlobal * StepTrash), 0.0f, 1.0f);
    self->ReturnIndicatorValue = Clamp(self->ReturnIndicatorValue + (ChangeGlobal * StepReturn), 0.0f, 1.0f);
    self->SubmitIndicatorValue = Clamp(self->SubmitIndicatorValue + (ChangeGlobal * StepSubmit), 0.0f, 1.0f);

    Vector2 PosMax = (Vector2){ .x = 0.0f, .y = 0.0f };
    Vector2 PosMinTrash = (Vector2) {.x = -0.5f, 0.0f };
    Vector2 PosMinReturn = (Vector2) {.x = 0.0f, -0.5f };
    Vector2 PosMinSubmit = (Vector2) {.x = 0.5f, 0.0f };

    Vector2 PosTrash = Vector2Lerp(PosMinTrash, PosMax, self->TrashIndicatorValue);
    Vector2 PosReturn = Vector2Lerp(PosMinReturn, PosMax, self->ReturnIndicatorValue);
    Vector2 PosSubmit = Vector2Lerp(PosMinSubmit, PosMax, self->SubmitIndicatorValue);

    Vector2 Origin = (Vector2) {.x = 0.0, .y = 0.0 };
    Vector2 Size = (Vector2) {.x = 1.0, .y = 1.0 };

    if (IsAtDesk)
    {
        Renderer_RenderTexture(renderer, assets->TrashIndicator, PosTrash, Size , Origin, 0.0f, WHITE, false, false);
        Renderer_RenderTexture(renderer, assets->SubmitIndicator, PosSubmit, Size, Origin, 0.0f, WHITE, false, false);
        Renderer_RenderTexture(renderer, assets->ReturnIndicator, PosReturn, Size, Origin, 0.0f, WHITE, false, false);
    }
}

static void DrawCheckPaper(MainGameContext* self, AssetCollection* assets, AhFuckRenderer* renderer)
{
    if (!IsNearDesk(self) || (self->CheckPaperState <= 0.0f))
    {
        return;
    }

    Vector2 PosMax = (Vector2){ .x = 0.5f, .y = 0.5f };
    Vector2 PosMin = (Vector2){ .x = PosMax.x, -PAPER_POS_DOWN.y };
    Vector2 Pos = Vector2Lerp(PosMin, PosMax, self->CheckPaperState);
    Vector2 Size = PAPER_SIZE_CHECK;
    Size.x *= PAPER_ASPECT_RATIO;

    Renderer_RenderTexture(renderer, assets->PaperGeneric, Pos, Size, (Vector2){ .x = 0.5, .y = 0.5 }, 0.0f, WHITE, true, false);
}

static void UpdateRoomAnimation(MainGameContext* self, float deltaTime)
{
    if (!self->RoomAnimationDirection)
    {
        return;
    }

    int32_t NormalizedDirection = self->RoomAnimationDirection > 0 ? 1 : -1;
    self->TimeSinceRoomAnimationUpdate += deltaTime;
    float TimePerFrame = ROOM_ANIMATION_DURATION_SECONDS / ROOM_ANIMATION_FRAME_COUNT;

    if (self->TimeSinceRoomAnimationUpdate <= TimePerFrame)
    {
        return;
    }

    const int32_t MinIndex = 0;
    const int32_t MaxIndex = ROOM_ANIMATION_FRAME_COUNT - 1;

    self->TimeSinceRoomAnimationUpdate -= TimePerFrame;
    int32_t NewIndex = self->AnimationIndex + NormalizedDirection;
    if (NewIndex > MaxIndex)
    {
        self->RoomAnimationDirection = 0;
        NewIndex = MaxIndex;
    }
    else if (NewIndex < MinIndex)
    {
        self->RoomAnimationDirection = 0;
        NewIndex = MinIndex;
    }
    self->AnimationIndex = NewIndex;
}

static Texture2D TextureOrEmpty(Texture2D texture, AssetCollection* assets)
{
    if (texture.id)
    {
        return texture;
    }
    return assets->EmptyTexture;
}

static void BeginDrawRoom(MainGameContext* self, AssetCollection* assets, AhFuckRenderer* renderer)
{
    Texture2D RoomTexture = TextureOrEmpty(assets->RoomAnimation[self->AnimationIndex], assets);
    Texture2D LightsTexture = TextureOrEmpty(assets->RoomLightAnimation[self->AnimationIndex], assets);

    Vector2 Position = (Vector2){.x = 0.5, .y = 0.5 };
    Vector2 Size = (Vector2){ .x = 1.0 * WINDOW_ASPECT_RATIO, .y = 1.0 };
    Vector2 Origin = (Vector2){ .x = 0.5, .y = 0.5 };

    SetShaderValueTexture(assets->InsideWorldShader, GetShaderLocation(assets->InsideWorldShader, "LightTexture"), LightsTexture);

    Renderer_RenderTexture(renderer, RoomTexture, Position, Size, Origin, 0.0f, WHITE, true, false);
}

static void EndDrawRoom(MainGameContext* self, AssetCollection* assets, AhFuckRenderer* renderer)
{
    Vector2 Position = (Vector2){.x = 0.5, .y = 0.5 };
    Vector2 Size = (Vector2){ .x = 1.0 * WINDOW_ASPECT_RATIO, .y = 1.0 };
    Vector2 Origin = (Vector2){ .x = 0.5, .y = 0.5 };

    Texture2D DayShadowTexture = TextureOrEmpty(assets->ShadowDayAnimation[self->AnimationIndex], assets);
    Texture2D NightShadowTexture = TextureOrEmpty(assets->ShadownNightAnimation[self->AnimationIndex], assets);

    float ShadowStrength = Clamp((self->DayTime * 2.0f), 0.0f, 1.0f);
    Color ShadowBrightness = (Color) 
    {
        .r = UINT8_MAX,
        .g = UINT8_MAX,
        .b = UINT8_MAX,
        .a = (uint8_t)(UINT8_MAX * (SHADOWN_BRIGHTNESS_MIN + (SHADOWN_BRIGHTNESS_MAX - SHADOWN_BRIGHTNESS_MIN) * ShadowStrength))
    };

    Texture2D ShadowTexture;
    if (self->DayTime <= NIGHT_SHADOW_ENABLE_TIME)
    {
        ShadowTexture = NightShadowTexture;
        ShadowBrightness.a = SHADOW_BRIGHTNESS_NIGHT;
    }
    else
    {    
        ShadowTexture = DayShadowTexture;
    }

    Renderer_RenderTexture(renderer, ShadowTexture, Position, Size, Origin, 0.0f, ShadowBrightness, true, false);
}

static void RenderPaperText(Vector2 position,
    Vector2 paperSize,
    Document* document,
    AssetCollection* assets,
    AhFuckRenderer* renderer,
    float rotation,
    Vector2 origin)
{
    const float FONT_SIZE = 0.015f;
    float MarginAmount = 0.05f;
    Vector2 Margin = Vector2Multiply(paperSize, (Vector2){ .x = MarginAmount, .y = MarginAmount * PAPER_ASPECT_RATIO });
    Vector2 Pos = Vector2Add(Vector2Subtract(position, Vector2Divide(paperSize, (Vector2) { .x = 2.0f, .y = 2.0f} )), Margin);
    if (document)
    {
        Renderer_RenderText(renderer,
            assets->MainFont,
            FONT_SIZE,
            Pos,
            origin,
            rotation,
            false,
            BLACK,
            document->Text);
    }
}

static void DrawPaper(MainGameContext* self, AssetCollection* assets, AhFuckRenderer* renderer)
{
    if (!IsNearDesk(self) || !self->IsPaperOnTable)
    {
        return;
    }

    Vector2 Position = self->PaperPosition;
    Vector2 ShadowPosition = Position;
    Rectangle PaperBounds = GetPaperBounds(self, renderer);
    Vector2 Size = (Vector2){ .x = PaperBounds.width, .y = PaperBounds.height };
    Vector2 Origin = (Vector2){ .x = 0.5, .y = 0.5 };

    const float PaperOffset = 0.015f;
    if (self->IsPaperSelected)
    {
        Position.x -= PaperOffset * Size.x * self->PaperHeight; 
        Position.y -= PaperOffset * Size.y * self->PaperHeight; 
    }

    Color ShadowColor = (Color){ .r = 0, .g = 0, .b = 0, .a = 200 };

    Renderer_RenderTexture(renderer, assets->PaperGeneric, ShadowPosition, Size, Origin, 0.0f, ShadowColor, false, false);
    Renderer_RenderTexture(renderer, assets->PaperGeneric, Position, Size, Origin, 0.0f, WHITE, false, false);
    RenderPaperText(Position, Size, self->ActiveDocument, assets, renderer, 0.0f, (Vector2){ .x = 0.0, .y = 0.0 });
}

static void DrawDocumentStack(MainGameContext* self, AssetCollection* assets, AhFuckRenderer* renderer)
{
    if (!IsNearDesk(self))
    {
        return;
    }

    Vector2 ChangePerPaper = Renderer_AdjustVector(renderer, (Vector2){ .x = 0.005, .y = -0.010 });
    
    Vector2 Size = PAPER_SIZE_DOWN;
    Size.x *= PAPER_ASPECT_RATIO;

    Vector2 StartPosition = DOCUMENT_POS_DOWN;

    for (size_t i = 0; i < self->DocumentCount; i++)
    {
        Document* TargetDoc = &self->Documents[i];

        Vector2 PaperOffset = Vector2Multiply(ChangePerPaper, (Vector2){ .x = i, .y = i });
        Vector2 Position = Vector2Add(StartPosition, PaperOffset);
        Position = Vector2Add(Position, Renderer_AdjustVector(renderer, TargetDoc->Offset));
        Vector2 ShadowPosition = Position;

        Vector2 Origin = (Vector2){ .x = 0.5, .y = 0.5 };

        if (i > 0)
        {
            const float ShadowOffset = 0.006f;
            ShadowPosition.x -= ShadowOffset * Size.x; 
            ShadowPosition.y += ShadowOffset * Size.y;
        }

        Color ShadowColor = (Color){ .r = 0, .g = 0, .b = 0, .a = 160 };

        
        float Rotation = TargetDoc->RotationDeg;

        Renderer_RenderTexture(renderer, assets->PaperGeneric, ShadowPosition, Size, Origin, Rotation, ShadowColor, true, false);
        Renderer_RenderTexture(renderer, assets->PaperGeneric, Position, Size, Origin, Rotation, WHITE, true, false);

        // Fuck this shit can't get it to work.
        // if (i == self->DocumentCount - 1)
        // {
        //     // Vector2 TopLeft = Vector2Subtract(Position, Vector2Divide(Size, (Vector2){ .x = 2.0f, .y = 2.0f}));
        //     // Vector2 CenterToPaper = Vector2Subtract(TopLeft, Position);
        //     // Vector2 RotatedVector = Vector2Rotate(CenterToPaper, -TargetDoc->RotationDeg * RAD2DEG);
        //     // printf("tl: %f %f\n", TopLeft.x, TopLeft.y);
        //     // printf("c: %f %f\n", CenterToPaper.x, CenterToPaper.y);
        //     // printf("r: %f %f\n", RotatedVector.x, RotatedVector.y);
        //     // printf("p: %f %f\n", Position.x, Position.y);

        //     //Vector2 PaperPos = Vector2Add(Position, RotatedVector);
        //     //printf("P: %f %f\n", PaperPos.x, PaperPos.y);
        //     RenderPaperText(StartPosition, Size, TargetDoc, assets, renderer, TargetDoc->RotationDeg, Origin);
        // }
    }
}

static void UpdateShaderValues(MainGameContext* self, AssetCollection* assets, AhFuckRenderer* renderer, float deltaTime)
{
    float Sanity = self->SanityFactor;

    int ColorStepCount = (int)roundf(COLOR_STEP_COUNT_MIN + ((COLOR_STEP_COUNT_MAX - COLOR_STEP_COUNT_MIN) * Sanity));
    float DepressionFactor = -Sanity + 1.0f;
    float DayBrightness = self->DayTime;
    Vector2 ScreenSize = renderer->WindowFloatSize;

    SetShaderValue(assets->GlobalShader, GetShaderLocation(assets->GlobalShader, "ScreenSize"), &ScreenSize, SHADER_UNIFORM_VEC2);
    SetShaderValue(assets->GlobalShader, GetShaderLocation(assets->GlobalShader, "DepressionFactor"), &DepressionFactor, SHADER_UNIFORM_FLOAT);
    SetShaderValue(assets->GlobalShader, GetShaderLocation(assets->GlobalShader, "ColorStepCount"), &ColorStepCount, SHADER_UNIFORM_INT);


    self->TimeSinceShaderRandomUpdate += deltaTime;
    if (self->TimeSinceShaderRandomUpdate >= (1.0f / SHADER_RANDOM_UPDATE_FREQUENCY))
    {
        self->TimeSinceShaderRandomUpdate = 0.0f;
        float RandomValue = GetRandomFloat();
        SetShaderValue(assets->GlobalShader, GetShaderLocation(assets->GlobalShader, "RandomValue"), &RandomValue, SHADER_UNIFORM_FLOAT);
    }

    SetShaderValue(assets->InsideWorldShader, GetShaderLocation(assets->InsideWorldShader, "DayBrightness"), &DayBrightness, SHADER_UNIFORM_FLOAT);
}

static void UpdateBackgroundColor(MainGameContext* self, AhFuckRenderer* renderer)
{
    float NightTimeStrength = Clamp(1.0f - (self->DayTime * 2.0f), 0.0f, 1.0f);
    float EveningTimeStrength = Clamp(AbsFloat(1.0f - self->DayTime) * 2.0f, 0.0f, 1.0f);

    renderer->ScreenClearColor = ColorLerp(ColorLerp(COLOR_DAY, COLOR_EVENING, EveningTimeStrength), COLOR_NIGHT, NightTimeStrength);
}

static Document* TakeDocumentFromSource(DocumentSource* source)
{
    Document* Target = &source->Documents[source->Count - 1];
    source->Count--;
    return Target;
}

static void BeginDay(MainGameContext* self, size_t averageDocumentCount, AudioContext* audio)
{
    if (averageDocumentCount == 0)
    {
        return;
    }

    size_t ClampedAverageDocumentCount = averageDocumentCount > MAX_DOCUMENTS ? MAX_DOCUMENTS : averageDocumentCount;

    const float MaxDocumentCountVariance = 0.1f;
    float DocumentCountVariance = ((GetRandomFloat() - 0.5f) * 2.0f);
    size_t DocumentCountMin = (size_t)MaxInt(1, (int)(ClampedAverageDocumentCount * (1.0f - MaxDocumentCountVariance)));
    size_t DocumentCountMax = (size_t)MinInt(MAX_DOCUMENTS, (int)(ClampedAverageDocumentCount * (1.0f + MaxDocumentCountVariance)));
    size_t DocumentCount = (size_t)(DocumentCountMin + ((DocumentCountMax - DocumentCountMin) * DocumentCountVariance));

    for (size_t i = 0; i < DocumentCount; i++)
    {
        if (self->AdsDocsSource.Count > 0)
        {
            AddDocument(self, TakeDocumentFromSource(&self->AdsDocsSource));
        }   
    }
    
    self->DayDocumentStartCount = self->DocumentCount;

    self->SanityFactor = 1.0f - ((float)DocumentCount / (float)MAX_DOCUMENTS);
    audio->AudioFuckShitUpAmount = 1.0f - self->SanityFactor;
}


// Functions.
void MainGame_CreateContext(MainGameContext* self, AhFuckContext* programContext)
{
    memset(self, 0, sizeof(*self));
    UNUSED(programContext);
}

void MainGame_Start(MainGameContext* self, AssetCollection* assets, AhFuckContext* programContext, AhFuckRenderer* renderer, AudioContext* audio)
{
    UNUSED(programContext);
    UNUSED(renderer);

    self->State = GameState_PreStart;
    self->SanityFactor = 1.0f;
    self->PreStartState.ElapsedStateDuration = 0.0f;
    self->DayTime = 1.0f;
    self->AnimationIndex = 0;
    self->TimeSinceShaderRandomUpdate = 0.0;
    self->TimeSinceRoomAnimationUpdate = 0.0f;
    self->IsPaperSelected = false;
    self->PaperPosition = PAPER_POS_DOWN;
    self->GameTime = 0.0f;
    self->IsPaperOnTable = false;
    self->Documents = MemAlloc(sizeof(Document) * MAX_DOCUMENTS);
    self->AdsDocsSource.Documents = MemAlloc(sizeof(Document) * MAX_DOCUMENTS);
    self->BlankDocsSource.Documents = MemAlloc(sizeof(Document) * MAX_DOCUMENTS);
    self->LegitForBenchSource.Documents = MemAlloc(sizeof(Document) * MAX_DOCUMENTS);
    self->LegitButNotForBenchSource.Documents = MemAlloc(sizeof(Document) * MAX_DOCUMENTS);
    self->ActiveDocument = NULL;
    self->CheckPaperState = 0.0f;
    InitDocuments(self);
    self->IsCheckingPaper = false;
    self->IsCameraMovementAllowed = true;
    self->Score = 0; // Initialize score
    
    BeginDay(self, 15, audio);

    renderer->GlobalLayer.IsShaderEnabled = true;
    renderer->GlobalLayer.TargetShader = assets->GlobalShader;
    renderer->GlobalScreenOpacity = 0.0f;

    renderer->WorldLayer.IsShaderEnabled = true;
    renderer->WorldLayer.TargetShader = assets->InsideWorldShader;
}

void MainGame_End(MainGameContext* self, AssetCollection* assets, AhFuckContext* programContext, AhFuckRenderer* renderer, AudioContext* audio)
{
    UNUSED(self);
    UNUSED(programContext);
    UNUSED(renderer);
    UNUSED(assets);
    UNUSED(audio);
    MemFree(self->Documents);
}

void MainGame_Update(MainGameContext* self,
    AssetCollection* assets,
    AhFuckContext* programContext,
    float deltaTime,
    AhFuckRenderer* renderer,
    AudioContext* audio)
{
    UNUSED(self);
    UNUSED(programContext);
    UNUSED(deltaTime);
    UNUSED(renderer);
    UNUSED(assets);

    UpdateBackgroundColor(self, renderer);
    UpdateRoomAnimation(self, deltaTime);

    switch (self->State)
    {
        case GameState_PreStart:
            PreStartUpdate(self, assets, deltaTime, renderer, audio);
            break;

        case GameState_InGame:
            InGameUpdate(self, assets, audio, deltaTime, renderer);
            break;
    
        default:
            break;
    }
}


void MainGame_Draw(MainGameContext* self, AssetCollection* assets, AhFuckContext* programContext, float deltaTime, AhFuckRenderer* renderer)
{
    UNUSED(programContext);
    UNUSED(deltaTime);

    UpdateShaderValues(self, assets, renderer, deltaTime);

    Renderer_BeginLayerRender(renderer, TargetRenderLayer_World);
    BeginDrawRoom(self, assets, renderer);
    DrawDocumentStack(self, assets, renderer);
    EndDrawRoom(self, assets, renderer);
    DrawPaper(self, assets, renderer);
    DrawCheckPaper(self, assets, renderer);
    Renderer_EndLayerRender(renderer);

    Renderer_BeginLayerRender(renderer, TargetRenderLayer_UI);
    DrawIndicators(self, assets, renderer, deltaTime);

    // Draw Score
    {
        char ScoreText[32];
        snprintf(ScoreText, sizeof(ScoreText), "Score: %d", self->Score);
        const float FONT_SIZE = 0.04f;
        Vector2 Pos = { 0.02f, 0.03f }; // Top-left corner
        Vector2 Origin = { 0.0f, 0.0f };
        Renderer_RenderText(renderer, assets->MainFont, FONT_SIZE, Pos, Origin, 0.0f, false, BLACK, ScoreText);
    }

    Renderer_EndLayerRender(renderer);
}

void MainGame_DestroyContext(MainGameContext* self, AhFuckContext* programContext)
{
    memset(self, 0, sizeof(*self));
    UNUSED(programContext);
}