#include "NetMenu.h"
#include "Netplay.h"

// courtesey of Eon
#define NETMENU_IMPL true

#if NETMENU_IMPL

// disable initial login attempt
// netThread/[NtMatching] near call to DWC_LoginAsync
INJECTION("setToLoggedIn", 0x8014B5F8, R"(
    li r4, 3
)");
INJECTION("setToLoggedIn2", 0x8014B5FC, R"(
    stw r4, -0x4048(r13)
)");

// NOTES on WIFI CSS/SSS:
// 0x80687f6c = Address where a value is checked, and if it's 1 then the timer is processed on the CSS
// 0x80687fe0 = Address where timer is converted to numbers before calling setCounter
// 0x80688010 = Address right after calling setCounter
// disable Set Counter original address is 800da6ec
// INJECTION("disableSetCounterOnCSS", 0x8068800c, "nop"); 
// Prevents the timer to decrease
// INJECTION("preventCounterSubsOnCSS", 0x80687f88, "mr r4, r3"); 
// create.muWifiCntWndTask.mu_wifi_cnt_wnd is the original method called to create the counter window

// Disables Creation of Timer HUD on CSS
INJECTION("disableCreateCounterOnCSS", 0x80686ae4, "BRANCH r3, 0x80686aec"); 

// Prevent Timer from being processed on CSS
INJECTION("turnOffCSSTimer", 0x80687f6c, "li r0, 0"); 

// Disables Creation of Timer HUD on SSS
INJECTION("disableCreateCounterOnSSS", 0x806b1da0, "BRANCH r3, 0x806B1DBC"); 

// Prevent Timer from being processed on SSS
INJECTION("turnOffSSSTimer", 0x806b3f28, "li r0, 8");

// disable Mii render func
INJECTION("disableMiiRender", 0x80033b48, "nop");

// TODO: 
// The following are being disabled because you get an error of 0xD when you go back in any of the scenes
// which shows the following dialog and drops you back to the main menu: 
// "Your connection to other players was lost. Returning to the menu."
// Ideally we should handle them better and figure out why we get that error on the first place.

// Disable getNetworkError from wifiInterface during CSS
INJECTION("disableGetNetworkErrorOnCSS", 0x80687c68, "li r3, 0"); // original was bl ->0x800CBCB0

// Disable getNetworkError from wifiInterface during SSS
INJECTION("disableGetNetworkErrorOnSSS", 0x806b3a74, "li r3, 0"); // original was bl ->0x800CBCB0


// disable error thrown on matchmaking
// startMatchingCommon/[muWifiInterfaceTask] forces branch to end of func
DATA_WRITE(0x800CCF70, 48000024); // <- "b 0x24"


// Training scene crap

// 806bea38 seems like this address requestPreloadNetworkCharacter loops through the 4 players to try to preload info on all chars
// 806beab4 callls getWifiPlayerInfo inside of the above method per each player, recevies r3-r7 r4 is the player index

// 80963d68 calls updateDispPlayerWifiMenuSeq is the original method that updates the display of the wifi menu for each player
// isCloseMatching checks if the matchmaking should end, if it returns true, everything is cleaned up and the menu is returned to

// 809644cc calls isCompleteMeleeSettingAllMember, return boolean (I think this checks if local player locked in with settings)
// 80964540 calls isCompleteCloseMatchingAllNode, returns boolean (I think this checks if all players have locked in)
// 8096466c calls isWifiPreloadCharacter, returns boolean (I think this checks if all players have preloaded their characters)
// 80964858 calls isPlayerAssignReceived, booleam (I think this checks if local player has received their assigned port)

// 80964960 calls startBeginMeleeAnimation, This is called on a second loop after everything above have returned true
// 8095f7f0 address of setPrepareLoadStageComplete, this is called after stage loads and melee has started on 806d2a74 inside scMelee.process I think
// 8095f8e4 address of isPreparedLoadStageEveryone, boolean and is invoked from 806d2ad4 in the same manner as above
// 8013fdd4 address of SetupIDList, I think this address sets up the list of player ids
// Training Scene crap ends


// Key parts to getting training mode to transition to melee
// 809644d0 return value needs to be true
// 80964670 return value needs to be true
// 8096485c return value needs to be true
// 8042ba8c address to batttlefield string


// start scmelee: 806d0148: on this address melee settings are set to process like an online match
// 806cf748 makigng this address follow the branch causes the scene to transition to the local melee scene

// pretend request of "ConnectToAnybodyAsync" succeeded
// thStartMatching/[NtMatching] replaces call to ConnectToAnybodyAsync with just a success code
INJECTION("ConnectToAnybodyAsyncHook", 0x801494A4, R"(
    li r3, 1
)");


// overrides the branch to thStartMatching
// called when you first get to the CSS in the online menu
SIMPLE_INJECTION(StartMatchingCallback, 0x8014aff8, "nop") {
    OSReport("Starting matchmaking!\n");
}

void ChangeGfSceneField(Scene scene) {
    u32* scenePtr = ((u32*)(((u8*)getGfSceneManager()) + 0x288));
    *scenePtr = (u32)scene;
    
    u32* somePtr = ((u32*)(((u8*)getGfSceneManager()) + 0x278));
    *somePtr = 0; // this is checked > -1 and if so will check the sceneManager scene for idle, if so, will changeNextScene
}
void ChangeStruct3Scenes(u8* structure, Scene scene, Scene nextScene) {
    u32* scenePtr = (u32*)(structure + 0x8);
    *scenePtr = scene;

    u32* nextScenePtr = (u32*)(structure + 0xc);
    *nextScenePtr = nextScene;
}
void ChangeStruct3Scenes(u8* structure, Scene scene) {
    u32* scenePtr = (u32*)(structure + 0x8);
    *scenePtr = scene;
}

void BootToScMelee() {
    OSReport("Booting to scMelee...\n");
    //setupMelee((void*)0x90ff42e0, 0);
    ChangeStruct3Scenes((u8*)0x90ff3e40, MemoryChange, InitialChange);
    //setNextSqVsMelee((void*)0x90ff42e0);
    //setNextSqNetAnyOkiraku((void*)0x90ff3e40);
    setNextScene(getGfSceneManager(), "scChallenger", 0);
    ChangeGfSceneField(Idle);
    changeNextScene(getGfSceneManager());
}

// setNext/[sqNetAnyOkiraku] at the top just before the do-while loop
// called just as we're loading into the online practice scene
SIMPLE_INJECTION(setNextAnyOkirakuTop, 0x806f2358, "li	r17, 1") {
    //BootToScMelee();

}

// at the bottom of the "case 5:" section of setNext/[sqNetAnyOkiraku]
// called just as we're loading into the online practice scene
SIMPLE_INJECTION(setNextAnyOkirakuCaseFive, 0x806f272c, "stw	r0, 0x000C (r15)") {
    OSReport("Loaded into online training room\n");
    //Netplay::StartMatching();
    //BootToScMelee();
}

// in netThread/[NtMatching] replaces call to netThreadTask with our own stuff
SIMPLE_INJECTION(netThreadTaskOverride, 0x8014b670, "nop") {
    //if (Netplay::IsInMatch()) { // if we are "in" the online training room
        //if (Netplay::CheckIsMatched()) {
        //    BootToScMelee();
            // Load into scMelee here or something
        //}
    //}

    //OSReport("setnextseq\n");
    //setNextSeq(getGfSceneManager(), "sqKumite", 0);
    //setNextScene(getGfSceneManager(), "scChallenger", 0);
    if (getCurrentFrame() == 150) {
        //BootToScMelee();
    }
}

#endif



namespace NetReport {
#define HOOK_NETREPORT true

#if HOOK_NETREPORT
// netReport/[muWifiInterface]
INJECTION("netReportHook", 0x800c7534, R"(
    SAVE_REGS
    bl netReportHook
    RESTORE_REGS

    addi	sp, sp, 112
)");


// netReport/[muWifiConnectWnd]
INJECTION("netReportHook2", 0x8119cd58, R"(
    SAVE_REGS
    bl netReportHook
    RESTORE_REGS

    stwu	sp, -0x0070 (sp)
)");

// netReport/[stOperatorNetwork]
INJECTION("netReportHook3", 0x8095f894, R"(
    SAVE_REGS
    bl netReportHook
    RESTORE_REGS

    stwu	sp, -0x0070 (sp)
)");


// netReport0/[ntMatch]
INJECTION("netReportHook4", 0x80147ec0, R"(
    SAVE_REGS
    bl netReportHook
    RESTORE_REGS

    stwu	sp, -0x0070 (sp)
)");

// netMinReport/[muWifiInterface]
INJECTION("netMinReportHook", 0x800c8f68, R"(
    SAVE_REGS
    bl netReportHook
    RESTORE_REGS

    stwu	sp, -0x0080 (sp)
)");



extern "C" void netReportHook(char* str) {
    OSReport(str);
}
#endif

}
