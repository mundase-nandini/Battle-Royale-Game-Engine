#include "rbg/Engine.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <cstdio>
#include <string>

namespace {
constexpr ImVec4 BG=ImVec4(.016f,.129f,.259f,1), PANEL=ImVec4(.082f,.235f,.416f,1);
constexpr ImVec4 PURPLE=ImVec4(.451f,.243f,.522f,1), PINK=ImVec4(.906f,.467f,.961f,1);
constexpr ImVec4 CREAM=ImVec4(.90f,.96f,1,1), MUTED=ImVec4(.52f,.70f,.82f,1);
constexpr ImVec4 CYAN=ImVec4(.239f,.878f,.988f,1);
int selected=-1; char newName[64]=""; bool showTrails=true; bool autoCombat=true; float simSpeed=1.0f; int newClass=0;
void theme(){
    ImGuiStyle& s=ImGui::GetStyle();s.WindowRounding=7;s.ChildRounding=6;s.FrameRounding=4;s.PopupRounding=5;s.ScrollbarRounding=6;s.GrabRounding=4;
    s.WindowPadding=ImVec2(12,10);s.FramePadding=ImVec2(8,5);s.ItemSpacing=ImVec2(8,7);
    auto& c=s.Colors;c[ImGuiCol_WindowBg]=BG;c[ImGuiCol_ChildBg]=ImVec4(.035f,.13f,.24f,1);c[ImGuiCol_PopupBg]=PANEL;
    c[ImGuiCol_Border]=ImVec4(.08f,.38f,.57f,1);c[ImGuiCol_FrameBg]=ImVec4(.04f,.16f,.29f,1);
    c[ImGuiCol_FrameBgHovered]=ImVec4(.14f,.38f,.58f,1);c[ImGuiCol_Button]=ImVec4(.20f,.38f,.58f,1);c[ImGuiCol_ButtonHovered]=PURPLE;
    c[ImGuiCol_ButtonActive]=PINK;c[ImGuiCol_Header]=PURPLE;c[ImGuiCol_HeaderHovered]=PINK;
    c[ImGuiCol_CheckMark]=CYAN;c[ImGuiCol_SliderGrab]=CYAN;c[ImGuiCol_Text]=CREAM;c[ImGuiCol_TextDisabled]=MUTED;
}
void heading(const char*t){ImGui::TextColored(CREAM,"%s",t);ImGui::Separator();}
void progress(const char*label,float v,float max,ImVec4 color){
    ImGui::Text("%s  %.0f / %.0f",label,v,max);ImGui::PushStyleColor(ImGuiCol_PlotHistogram,color);
    ImGui::ProgressBar(max>0?v/max:0,ImVec2(-1,8),"");ImGui::PopStyleColor();
}
void battlefield(rbg::Match&m){
    ImVec2 avail=ImGui::GetContentRegionAvail();avail.y=std::max(240.0f,avail.y);
    ImVec2 origin=ImGui::GetCursorScreenPos();ImDrawList*d=ImGui::GetWindowDrawList();
    d->AddRectFilledMultiColor(origin,ImVec2(origin.x+avail.x,origin.y+avail.y),IM_COL32(4,33,66,255),IM_COL32(21,60,106,255),IM_COL32(10,25,58,255),IM_COL32(7,18,43,255));
    d->PushClipRect(origin,ImVec2(origin.x+avail.x,origin.y+avail.y),true);
    const float side=std::min(avail.x,avail.y),ox=origin.x+(avail.x-side)*.5f,oy=origin.y+(avail.y-side)*.5f;
    for(int i=0;i<=10;++i){float p=side*i/10.0f;d->AddLine(ImVec2(ox+p,oy),ImVec2(ox+p,oy+side),IM_COL32(27,117,160,95));d->AddLine(ImVec2(ox,oy+p),ImVec2(ox+side,oy+p),IM_COL32(27,117,160,95));}
    float radius=side*m.zone().radius()/100.0f;
    d->AddCircle(ImVec2(ox+side*.5f,oy+side*.5f),radius,IM_COL32(61,224,252,230),100,2.5f);
    d->AddCircleFilled(ImVec2(ox+side*.5f,oy+side*.5f),radius,IM_COL32(120,75,110,18),100);
    d->AddText(ImVec2(ox+8,oy+8),IM_COL32(246,219,192,210),"TACTICAL MAP  /  2.5D SIMULATION");
    for(const auto&p:m.players()){
        if(showTrails && p.trail.size()>1 && p.alive){
            for(std::size_t k=1;k<p.trail.size();++k){
                float ax=ox+side*p.trail[k-1].x/100.0f, ay=oy+side*p.trail[k-1].y/100.0f;
                float bx=ox+side*p.trail[k].x/100.0f, by=oy+side*p.trail[k].y/100.0f;
                d->AddLine(ImVec2(ax,ay),ImVec2(bx,by),IM_COL32(61,224,252,static_cast<int>(35+190.0f*k/p.trail.size())),2.0f);
            }
        }
        float x=ox+side*p.position.x/100.0f,y=oy+side*p.position.y/100.0f;
        ImU32 col=!p.alive?IM_COL32(115,105,115,255):(p.id==static_cast<rbg::PlayerId>(selected)?IM_COL32(255,218,170,255):IM_COL32(61,224,252,255));
        d->AddCircleFilled(ImVec2(x,y),10,col,24);d->AddCircle(ImVec2(x,y),13,IM_COL32(245,220,200,190),24,1.5f);
        char label[96];std::snprintf(label,sizeof(label),"%s  %dHP",p.name.c_str(),(int)p.health);
        d->AddText(ImVec2(x+14,y-7),p.alive?IM_COL32(250,235,220,255):IM_COL32(150,140,150,255),label);
    }
    d->PopClipRect();ImGui::Dummy(avail);
}
void render(rbg::Match&m,float dt){
    ImGui::SetNextWindowPos(ImVec2(0,0),ImGuiCond_Always);ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize,ImGuiCond_Always);
    ImGui::Begin("RBG COMMAND",nullptr,ImGuiWindowFlags_NoDecoration|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoSavedSettings|ImGuiWindowFlags_MenuBar);
    { ImVec2 a=ImGui::GetWindowPos(), z=ImGui::GetWindowSize(); ImGui::GetWindowDrawList()->AddRectFilledMultiColor(a,ImVec2(a.x+z.x,a.y+z.y),IM_COL32(4,33,66,255),IM_COL32(21,60,106,255),IM_COL32(5,22,48,255),IM_COL32(7,29,57,255)); }
    if(ImGui::BeginMenuBar()){ImGui::TextColored(CREAM,"RBG ENGINE");ImGui::SameLine();ImGui::TextDisabled("  ROYAL BATTLE GAME  /  TACTICAL COMMAND");ImGui::EndMenuBar();}
    ImGui::BeginChild("toolbar",ImVec2(0,48),true);
    { ImVec2 a=ImGui::GetCursorScreenPos(); ImVec2 z=ImGui::GetContentRegionAvail(); ImGui::GetWindowDrawList()->AddRectFilledMultiColor(a,ImVec2(a.x+z.x,a.y+38),IM_COL32(36,100,151,210),IM_COL32(115,62,133,210),IM_COL32(115,62,133,210),IM_COL32(36,100,151,210)); }
    if(m.state()==rbg::MatchState::Running){if(ImGui::Button("PAUSE"))m.pause();}else{if(ImGui::Button("START / RESUME"))m.start();}
    ImGui::SameLine();if(ImGui::Button("STEP")){if(m.state()==rbg::MatchState::Paused){m.resume();m.tick(.25f);m.pause();}}
    ImGui::SameLine();if(ImGui::Button("RESET"))m.reset();ImGui::SameLine();ImGui::Checkbox("AUTO COMBAT",&autoCombat);m.setAutoCombat(autoCombat);ImGui::SameLine();ImGui::Checkbox("TRAILS",&showTrails);
    ImGui::SameLine();ImGui::SetNextItemWidth(110);ImGui::SliderFloat("SPEED",&simSpeed,.25f,3.0f,"%.2fx");
    ImGui::SameLine();ImGui::TextColored(PINK,"● %s",rbg::toString(m.state()));
    ImGui::EndChild();
    float h=ImGui::GetContentRegionAvail().y;float top=h*.69f;
    ImGui::BeginChild("mainrow",ImVec2(0,top),false);
    float totalW=ImGui::GetContentRegionAvail().x; bool compact=totalW<1120.0f; float left=compact?totalW:totalW*.66f;
    ImGui::BeginChild("battlefield",ImVec2(left,compact?ImGui::GetContentRegionAvail().y*.58f:0),true);heading("BATTLEFIELD  /  LIVE WORLD");battlefield(m);ImGui::EndChild();if(!compact)ImGui::SameLine();
    ImGui::BeginChild("right",ImVec2(0,0),false);
    ImGui::BeginChild("roster",ImVec2(0,ImGui::GetContentRegionAvail().y*.52f),true);heading("PLAYER ROSTER");
    ImGui::Text("%d / %zu PLAYERS  |  %d ALIVE",(int)m.players().size(),m.capacity(),m.aliveCount());
    for(const auto&p:m.players()){char b[128];std::snprintf(b,sizeof(b),"%s  [%s]##%u",p.name.c_str(),p.alive?"ALIVE":"OUT",p.id);
        if(ImGui::Selectable(b,selected==(int)p.id))selected=(int)p.id;}
    ImGui::EndChild();
    ImGui::BeginChild("inspector",ImVec2(0,0),true);heading("PLAYER INSPECTOR");rbg::Player* p=m.find((rbg::PlayerId)selected);
    if(p){ImGui::Text("%s  /  ID %u",p->name.c_str(),p->id);ImGui::Text("%s  |  %s",rbg::className(p->playerClass),rbg::abilityName(p->playerClass));progress("HEALTH",p->health,p->maxHealth,ImVec4(.65f,.25f,.38f,1));progress("SHIELD",p->shield,p->maxShield,ImVec4(.42f,.32f,.68f,1));
        ImGui::Text("Armour  %.0f   Eliminations  %d   Damage dealt  %.0f",p->armor,p->eliminations,p->damageDealt);ImGui::ProgressBar(p->abilityCharge/100.0f,ImVec2(-1,7),"ABILITY CHARGE");
        const char* names[5];for(int i=0;i<5;++i)names[i]=m.weapons()[i].name.c_str();int wi=p->weaponId-1;
        if(ImGui::Combo("Specialist weapon",&wi,names,5))m.assignWeapon(p->id,wi+1);
        ImGui::DragFloat("X",&p->position.x,.5f,0,100);ImGui::DragFloat("Y",&p->position.y,.5f,0,100);ImGui::DragFloat("Height",&p->position.z,.1f,-10,20);
        ImGui::DragFloat("Armour",&p->armor,.5f,0,100);
    }else ImGui::TextDisabled("Select a player from the roster.");
    ImGui::EndChild();ImGui::EndChild();ImGui::EndChild();
    ImGui::BeginChild("bottom",ImVec2(0,0),true);
    float bw=ImGui::GetContentRegionAvail().x*.35f;
    ImGui::BeginChild("addplayer",ImVec2(bw,0),false);heading("PLAYER / LOADOUT MANAGEMENT");
    ImGui::SetNextItemWidth(130);ImGui::InputText("Name",newName,sizeof(newName));ImGui::SameLine();
    const char* classOptions[]={"Vanguard / Assault","Phantom / Recon","Titan / Sentinel","Arcweaver / Medic"};
    ImGui::SetNextItemWidth(180);ImGui::Combo("Class",&newClass,classOptions,4);ImGui::SameLine();
    if(ImGui::Button("ADD PLAYER")){std::string n=newName[0]?newName:"New Player";auto*q=m.addPlayer(n,static_cast<rbg::PlayerClass>(newClass));if(q)selected=(int)q->id;newName[0]='\0';}
    ImGui::TextColored(CYAN,"SPECIALIST ARCHETYPES");
    ImGui::BulletText("VANGUARD  /  110 HP  /  Royal Assault Rifle  /  OVERDRIVE");
    ImGui::BulletText("PHANTOM  /  85 HP  /  Longshot Sniper  /  PHASE DASH");
    ImGui::BulletText("TITAN  /  145 HP  /  Titan LMG  /  FORTIFY");
    ImGui::BulletText("ARCWEAVER  /  100 HP  /  Violet Repeater  /  NOVA PULSE");
    ImGui::TextDisabled("Class choice sets weapon, health, shield, armour and movement profile.");
    ImGui::Text("Safe zone phase %d  |  Radius %.1f  |  Storm %.1f DPS",m.zone().phase(),m.zone().radius(),m.zone().damagePerSecond());
    ImGui::EndChild();ImGui::SameLine();
    ImGui::BeginChild("eventlog",ImVec2(0,0),false);heading("COMBAT FEED / EVENT HISTORY");
    const auto& ev=m.log().events();ImGui::BeginChild("events_scroll",ImVec2(0,0),false);
    for(auto it=ev.rbegin();it!=ev.rend();++it){ImVec4 c=it->category=="ELIMINATION"?PINK:(it->category=="COMBAT"?CREAM:MUTED);
        ImGui::TextColored(c,"[%06.1fs] %-12s %s",it->time,it->category.c_str(),it->message.c_str());}
    if(ImGui::GetScrollY()>=ImGui::GetScrollMaxY())ImGui::SetScrollHereY(1.0f);ImGui::EndChild();ImGui::EndChild();
    ImGui::EndChild();ImGui::End();
    if(m.state()==rbg::MatchState::Running)m.tick(dt*simSpeed);
}
}
int main(){
    if(!glfwInit())return 1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,0);
    GLFWwindow*w=glfwCreateWindow(1600,960,"RBG ENGINE | ROYAL BATTLE COMMAND",nullptr,nullptr);
    if(!w){glfwTerminate();return 2;}glfwMakeContextCurrent(w);glfwSwapInterval(1);
    IMGUI_CHECKVERSION();ImGui::CreateContext();ImGui::StyleColorsDark();theme();
    ImGui_ImplGlfw_InitForOpenGL(w,true);ImGui_ImplOpenGL3_Init("#version 130");
    rbg::Engine engine;engine.initialize();auto&m=engine.createMatch(20);
    auto* first=m.addPlayer("Viper",rbg::PlayerClass::Assault);m.addPlayer("Ghost",rbg::PlayerClass::Recon);m.addPlayer("Shadow",rbg::PlayerClass::Sentinel);m.addPlayer("Nova",rbg::PlayerClass::Medic);m.addPlayer("Rook",rbg::PlayerClass::Assault);if(first)selected=static_cast<int>(first->id);
    bool running=true;while(!glfwWindowShouldClose(w)&&running){glfwPollEvents();int width,height;glfwGetFramebufferSize(w,&width,&height);
        ImGui_ImplOpenGL3_NewFrame();ImGui_ImplGlfw_NewFrame();ImGui::NewFrame();
        render(m,ImGui::GetIO().DeltaTime);
        ImGui::Render();glViewport(0,0,width,height);glClearColor(BG.x,BG.y,BG.z,1);glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());glfwSwapBuffers(w);}
    ImGui_ImplOpenGL3_Shutdown();ImGui_ImplGlfw_Shutdown();ImGui::DestroyContext();glfwDestroyWindow(w);glfwTerminate();engine.shutdown();return 0;
}
