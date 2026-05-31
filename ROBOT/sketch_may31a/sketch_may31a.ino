// ================================================================
//  VECTOR FACE V8.2 - PET ROBOT EDITION
//  Tích hợp: L298N dual motor + AI né vật cản + né vực thẳm
// ================================================================

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// --- CẢM BIẾN PHÍA TRƯỚC ---
#define TRIG_PIN 2
#define ECHO_PIN 3

// --- CẢM BIẾN PHÍA DƯỚI (NÉ VỰC) ---
#define TRIG_FLOOR 4
#define ECHO_FLOOR 5

// ================================================================
// L298N MOTOR DRIVER PINS
// ================================================================
// Motor A (trái)
#define IN1 6
#define IN2 7
#define ENA 9   // PWM

// Motor B (phải)
#define IN3 10
#define IN4 11
#define ENB 8  // PWM (dùng pin 8, không phải PWM thật -> thay bằng 11 nếu muốn speed control)

// Tốc độ mặc định
#define SPEED_NORMAL 180
#define SPEED_SLOW   120
#define SPEED_TURN   160

// ================================================================
// TRẠNG THÁI ROBOT
// ================================================================
#define STATE_ROAM      0   // Tự đi chơi
#define STATE_AVOID     1   // Né vật cản phía trước
#define STATE_CLIFF     2   // Phát hiện vực, lùi + quay
#define STATE_INTERACT  3   // Có người tương tác gần (< 50cm) -> đứng yên / nhảy

byte robot_state = STATE_ROAM;
long stateTimer = 0;        // Thời điểm vào trạng thái hiện tại
int roam_action = 0;        // Hành động đang làm khi roam
long roamActionTimer = 0;

// ================================================================
// [GIỮ NGUYÊN] Tất cả biến gốc
// ================================================================
float cur_L_x = 24, cur_L_y = 20, cur_L_w = 30, cur_L_h = 24;
float tar_L_x = 24, tar_L_y = 20, tar_L_w = 30, tar_L_h = 24;
float cur_R_x = 74, cur_R_y = 20, cur_R_w = 30, cur_R_h = 24;
float tar_R_x = 74, tar_R_y = 20, tar_R_w = 30, tar_R_h = 24;
float cur_L_angle = 0, tar_L_angle = 0;
float cur_R_angle = 0, tar_R_angle = 0;
float cur_Bottom_cut = 0, tar_Bottom_cut = 0;

int current_emotion = 0;
int last_emotion = 0;
float animSpeed = 0.35;
long timerIdle = 0;
int frameCount = 0;

long lastInteractTime = 0;
int random_alone_emotion = 0;
int id_cau_noi = 0;
long timerTalk = 0;
long lockTimer = 0;

String getCauNoi(int id) {
  switch(id) {
    case 0: return F("Chao cau nha! ^^");
    case 1: return F("Nay on khong? O_o");
    case 2: return F("Di ngu thoi -.-");
    case 3: return F("Troi mat me qua~");
    case 4: return F("Toi doi qua... T_T");
    case 5: return F("Nhin gi vay? >_<");
    case 6: return F("Choi game ko? :D");
    default: return F("Hi! =]]");
  }
}

void setEmotion_For_Sentence(int id) {
  switch(id) {
    case 0: setEmotion_Happy(); break;
    case 1: setEmotion_Thinking(); break;
    case 2: setEmotion_Bored(); break;
    case 3: setEmotion_Idle(); break;
    case 4: setEmotion_Sad(); break;
    case 5: setEmotion_Angry(); break;
    case 6: setEmotion_Wink(); break;
    default: setEmotion_Talking(); break;
  }
}

// ================================================================
// [GIỮ NGUYÊN] MULTI-GAME ENGINE
// ================================================================
byte active_game = 0;
uint16_t t_board[15];
int t_px = 3, t_py = 0, t_piece = 0, t_target_px = 3;
const uint16_t shapes[7] = { 0x0F00, 0x0660, 0x0E40, 0x06C0, 0x0C60, 0x08E0, 0x02E0 };
int s_hx = 5, s_hy = 7, s_fx = 2, s_fy = 10;
byte s_tailX[15], s_tailY[15];
int s_len = 3, s_score = 0;
float p_bx = 64, p_by = 32, p_dx = 1.8, p_dy = 1.2;
float p_padY = 32;

// ================================================================
// MOTOR FUNCTIONS
// ================================================================
void motorStop() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
  analogWrite(ENA, 0);    analogWrite(ENB, 0);
}

void motorForward(int speed) {
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
  analogWrite(ENA, speed); analogWrite(ENB, speed);
}

void motorBackward(int speed) {
  digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
  analogWrite(ENA, speed); analogWrite(ENB, speed);
}

void motorTurnLeft(int speed) {
  // Motor trái lùi, phải tiến -> quay trái tại chỗ
  digitalWrite(IN1, LOW);  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
  analogWrite(ENA, speed); analogWrite(ENB, speed);
}

void motorTurnRight(int speed) {
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);  digitalWrite(IN4, HIGH);
  analogWrite(ENA, speed); analogWrite(ENB, speed);
}

// ================================================================
// CẢM BIẾN
// ================================================================
long docKhoangCach() {
  digitalWrite(TRIG_PIN, LOW); delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH); delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long dur = pulseIn(ECHO_PIN, HIGH, 30000);
  return (dur == 0) ? 100 : (dur / 2) / 29.1;
}

long docKhoangCachSan() {
  // Cảm biến hướng xuống đất, nếu khoảng cách > ngưỡng -> vực!
  digitalWrite(TRIG_FLOOR, LOW); delayMicroseconds(2);
  digitalWrite(TRIG_FLOOR, HIGH); delayMicroseconds(10);
  digitalWrite(TRIG_FLOOR, LOW);
  long dur = pulseIn(ECHO_FLOOR, HIGH, 30000);
  return (dur == 0) ? 100 : (dur / 2) / 29.1;
}

// ================================================================
// ROBOT AI - LOGIC DI CHUYỂN
// ================================================================

// Robot tự đi chơi: đi thẳng, ngoặt ngẫu nhiên, nhảy...
void updateRoam() {
  long now = millis();
  if (now - roamActionTimer > 2000) { // Cứ 2 giây đổi hành động
    roam_action = random(0, 5);
    roamActionTimer = now;
  }
  switch(roam_action) {
    case 0: motorForward(SPEED_NORMAL); break;    // Đi thẳng
    case 1: motorTurnLeft(SPEED_TURN);  break;    // Quẹo trái
    case 2: motorTurnRight(SPEED_TURN); break;    // Quẹo phải
    case 3: motorStop(); break;                   // Dừng ngơ ngác
    case 4: motorForward(SPEED_SLOW); break;      // Đi chậm ngắm nghía
  }
}

void updateRobotAI(long distFront, long distFloor) {
  long now = millis();

  // --- ƯU TIÊN 1: PHÁT HIỆN VỰC THẲM ---
  // Cảm biến sàn: bình thường ~3-5cm, nếu >12cm => HẾT ĐẤT!
  if (distFloor > 12) {
    if (robot_state != STATE_CLIFF) {
      robot_state = STATE_CLIFF;
      stateTimer = now;
      setEmotion_Surprised(); // Hoảng sợ!
    }
    // Pha 1 (0-600ms): Lùi nhanh
    if (now - stateTimer < 600) {
      motorBackward(SPEED_NORMAL);
    }
    // Pha 2 (600-1400ms): Quay ngẫu nhiên
    else if (now - stateTimer < 1400) {
      if (random(0,2) == 0) motorTurnLeft(SPEED_TURN);
      else motorTurnRight(SPEED_TURN);
    }
    // Pha 3: Về roam
    else {
      robot_state = STATE_ROAM;
    }
    return;
  }

  // --- ƯU TIÊN 2: VẬT CẢN PHÍA TRƯỚC ---
  if (distFront > 0 && distFront <= 20) {
    if (robot_state != STATE_AVOID) {
      robot_state = STATE_AVOID;
      stateTimer = now;
    }
    // Nhìn góc nghiêng thể hiện bực bội / hoảng
    if (distFront <= 10) setEmotion_Angry();
    else setEmotion_Surprised();

    // Pha 1: Lùi
    if (now - stateTimer < 500) {
      motorBackward(SPEED_NORMAL);
    }
    // Pha 2: Quay tránh (random trái/phải)
    else if (now - stateTimer < 1100) {
      if ((stateTimer % 2) == 0) motorTurnLeft(SPEED_TURN);
      else motorTurnRight(SPEED_TURN);
    }
    else {
      robot_state = STATE_ROAM;
    }
    return;
  }

  // --- ƯU TIÊN 3: CÓ NGƯỜI TỪ 20-50CM -> Tò mò, đứng im chào ---
  if (distFront > 20 && distFront <= 50) {
    robot_state = STATE_INTERACT;
    motorStop();
    // Cảm xúc được xử lý bởi loop chính (giữ nguyên logic gốc)
    return;
  }

  // --- MẶC ĐỊNH: ROAM TỰ DO ---
  robot_state = STATE_ROAM;
  updateRoam();
}

// ================================================================
// SETUP
// ================================================================
void setup() {
  Serial.begin(9600);

  // Motor pins
  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT); pinMode(ENA, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT); pinMode(ENB, OUTPUT);
  motorStop();

  // Cảm biến
  pinMode(TRIG_PIN, OUTPUT); pinMode(ECHO_PIN, INPUT);
  pinMode(TRIG_FLOOR, OUTPUT); pinMode(ECHO_FLOOR, INPUT);

  Wire.begin();
  Wire.setClock(400000L);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("OLED FAIL!")); for(;;);
  }

  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(10, 20);
  display.print(F("PET ROBOT ONLINE~"));
  display.setCursor(20, 36);
  display.print(F("Lets play! :3"));
  display.display();
  delay(1500);

  lastInteractTime = millis();
}

// ================================================================
// LOOP CHÍNH (Logic mặt giữ nguyên 100%, thêm motor AI)
// ================================================================
void loop() {
  frameCount++;
  long distance = docKhoangCach();
  long distFloor = docKhoangCachSan();
  last_emotion = current_emotion;

  // ============================================================
  // PHẦN MỚI: Gọi AI motor (chạy song song với logic mặt)
  // Logic mặt vẫn ưu tiên hoạt động như cũ, motor chạy độc lập
  // ============================================================
  updateRobotAI(distance, distFloor);

  // ============================================================
  // [GIỮ NGUYÊN HOÀN TOÀN] Logic biểu cảm gốc
  // ============================================================
  if (millis() < lockTimer) {
    setEmotion_Talking();
  }
  else if (distance <= 50) {
    lastInteractTime = millis();
    if (distance > 30 && distance <= 50) {
      if (frameCount % 200 == 0) {
        int r = random(0, 4);
        if (r == 0) setEmotion_Idle();
        else if (r == 1) setEmotion_Thinking();
        else if (r == 2) setEmotion_Wink();
        else setEmotion_Bored();
      }
      current_emotion = 2;
    }
    else if (distance > 22 && distance <= 30) { setEmotion_Happy(); current_emotion = 3; }
    else if (distance > 15 && distance <= 22) { setEmotion_Surprised(); current_emotion = 7; }
    else if (distance > 10 && distance <= 15) { setEmotion_Music(); current_emotion = 5; }
    else if (distance > 6  && distance <= 10) { setEmotion_Angry(); current_emotion = 6; }
    else {
      current_emotion = 9;
      if (last_emotion != 9) switchGameRandom();
    }
  }
  else {
    long timeAlone = millis() - lastInteractTime;
    if (timeAlone < 8000) {
      setEmotion_Sleep(); current_emotion = 1;
    }
    else if (timeAlone < 13000) {
      if (current_emotion == 1) {
        random_alone_emotion = random(0, 12);
        if (random_alone_emotion >= 4) {
          current_emotion = 10;
          id_cau_noi = random(0, 7);
          timerTalk = millis();
          lockTimer = millis() + 6000;
        } else {
          current_emotion = 8;
        }
      }
      if (current_emotion == 10) {
        setEmotion_For_Sentence(id_cau_noi);
      } else {
        if (random_alone_emotion == 0) setEmotion_Surprised();
        else if (random_alone_emotion == 1) setEmotion_Thinking();
        else if (random_alone_emotion == 2) setEmotion_Wink();
        else setEmotion_Bored();
      }
    }
    else {
      lastInteractTime = millis();
    }
  }

  // [GIỮ NGUYÊN] Game AI
  if (current_emotion == 9) {
    if (active_game == 0) updateTetris();
    else if (active_game == 1) updateSnake();
    else updatePong();
    setEmotion_Gaming();
  }

  // [GIỮ NGUYÊN] LERP Engine
  cur_L_x += (tar_L_x - cur_L_x) * animSpeed; cur_L_y += (tar_L_y - cur_L_y) * animSpeed;
  cur_L_w += (tar_L_w - cur_L_w) * animSpeed; cur_L_h += (tar_L_h - cur_L_h) * animSpeed;
  cur_R_x += (tar_R_x - cur_R_x) * animSpeed; cur_R_y += (tar_R_y - cur_R_y) * animSpeed;
  cur_R_w += (tar_R_w - cur_R_w) * animSpeed; cur_R_h += (tar_R_h - cur_R_h) * animSpeed;
  cur_L_angle += (tar_L_angle - cur_L_angle) * animSpeed;
  cur_R_angle += (tar_R_angle - cur_R_angle) * animSpeed;
  cur_Bottom_cut += (tar_Bottom_cut - cur_Bottom_cut) * animSpeed;

  // [GIỮ NGUYÊN] Render
  veMatVector();
  delay(15);
}

// ================================================================
// [GIỮ NGUYÊN HOÀN TOÀN] Tất cả hàm gốc bên dưới
// ================================================================

void switchGameRandom() {
  active_game = random(0, 3);
  if(active_game == 0) { for(int i=0; i<15; i++) t_board[i] = 0; t_py = 0; t_px = 3; }
  else if(active_game == 1) { s_len = 3; s_score = 0; s_hx = 5; s_hy = 7; s_fx = random(1,9); s_fy = random(1,13); }
  else { p_bx = 64; p_by = 32; p_dx = (random(0,2) == 0) ? 1.8 : -1.8; }
}

bool checkCollision(int nx, int ny) {
  for(int r=0; r<4; r++) { for(int c=0; c<4; c++) {
    if((shapes[t_piece] >> (15 - (r*4+c))) & 1) {
      int bx = nx + c, by = ny + r;
      if(bx < 0 || bx >= 10 || by >= 15) return true;
      if(by >= 0 && (t_board[by] & (1<<bx))) return true;
    }
  }} return false;
}
void mergePiece() {
  for(int r=0; r<4; r++) { for(int c=0; c<4; c++) {
    if((shapes[t_piece] >> (15 - (r*4+c))) & 1) {
      int bx = t_px + c, by = t_py + r;
      if(by >= 0 && by < 15) t_board[by] |= (1<<bx);
    }
  }}
}
void clearLines() {
  for(int r=14; r>=0; r--) {
    if((t_board[r] & 0x03FF) == 0x03FF) {
      for(int k=r; k>0; k--) t_board[k] = t_board[k-1];
      t_board[0] = 0; r++;
    }
  }
}
void updateTetris() {
  if(frameCount % 2 == 0) {
    if(t_px < t_target_px && !checkCollision(t_px+1, t_py)) t_px++;
    else if(t_px > t_target_px && !checkCollision(t_px-1, t_py)) t_px--;
  }
  if(frameCount % 3 == 0) {
    if(!checkCollision(t_px, t_py+1)) { t_py++; }
    else { mergePiece(); clearLines(); t_piece = random(0, 7); t_px = 3; t_py = 0; t_target_px = random(-1, 8); if(checkCollision(t_px, t_py)) switchGameRandom(); }
  }
}
void drawTetris() {
  for(int r=0; r<15; r++) { for(int c=0; c<10; c++) { if(t_board[r] & (1<<c)) display.fillRect(44 + c*4, 4 + r*4, 3, 3, WHITE); } }
  for(int r=0; r<4; r++) { for(int c=0; c<4; c++) { if((shapes[t_piece] >> (15 - (r*4+c))) & 1) display.fillRect(44 + (t_px+c)*4, 4 + (t_py+r)*4, 3, 3, WHITE); } }
}
void updateSnake() {
  if(frameCount % 4 == 0) {
    for(int i = s_len - 1; i > 0; i--) { s_tailX[i] = s_tailX[i-1]; s_tailY[i] = s_tailY[i-1]; }
    s_tailX[0] = s_hx; s_tailY[0] = s_hy;
    if (s_hx < s_fx) s_hx++; else if (s_hx > s_fx) s_hx--; else if (s_hy < s_fy) s_hy++; else if (s_hy > s_fy) s_hy--;
    if(s_hx == s_fx && s_hy == s_fy) { if(s_len < 14) s_len++; s_score++; s_fx = random(1, 9); s_fy = random(1, 13); if(s_score > 8) switchGameRandom(); }
    if(s_hx < 0 || s_hx > 10 || s_hy < 0 || s_hy > 14) switchGameRandom();
  }
}
void drawSnake() {
  if(frameCount % 4 < 2) display.fillRect(44 + s_fx*4, 4 + s_fy*4, 3, 3, WHITE);
  display.fillRect(44 + s_hx*4, 4 + s_hy*4, 3, 3, WHITE);
  for(int i=0; i<s_len; i++) display.fillRect(44 + s_tailX[i]*4, 4 + s_tailY[i]*4, 2, 2, WHITE);
}
void updatePong() {
  p_bx += p_dx; p_by += p_dy;
  if(p_by <= 4 || p_by >= 60) p_dy = -p_dy;
  if(p_bx <= 45) { p_dx = -p_dx; p_bx = 45; }
  if(p_bx >= 81 && p_bx <= 83 && abs(p_by - p_padY) < 8) { p_dx = -p_dx; p_bx = 80; }
  else if (p_bx > 86) switchGameRandom();
  p_padY += (p_by - p_padY) * 0.25;
  if(p_padY < 8) p_padY = 8; if(p_padY > 56) p_padY = 56;
}
void drawPong() {
  display.fillRect((int)p_bx, (int)p_by, 3, 3, WHITE);
  display.fillRect(83, (int)p_padY - 6, 2, 12, WHITE);
}

void setEmotion_Talking() { tar_L_w=20; tar_L_h=20; tar_L_x=24; tar_L_y=36; tar_R_w=20; tar_R_h=20; tar_R_x=84; tar_R_y=36; tar_L_angle=0; tar_R_angle=0; tar_Bottom_cut=0; }
void setEmotion_Gaming() {
  tar_L_w=12; tar_L_h=24; tar_L_x=18; tar_R_w=12; tar_R_h=24; tar_R_x=98;
  tar_L_angle=0; tar_R_angle=0; tar_Bottom_cut=0;
  int eyeY = 20;
  if (active_game == 0) eyeY = 10 + t_py * 2;
  else if (active_game == 1) eyeY = 10 + s_hy * 2;
  else if (active_game == 2) eyeY = p_by - 12;
  if(eyeY < 4) eyeY = 4; if(eyeY > 38) eyeY = 38; tar_L_y = eyeY; tar_R_y = eyeY;
}
void setEmotion_Sleep() {
  tar_L_w=30; tar_L_h=4; tar_L_x=24; tar_R_w=30; tar_R_h=4; tar_R_x=74;
  tar_L_angle=0; tar_R_angle=0; tar_Bottom_cut=0;
  if (frameCount % 80 < 40) { tar_L_y=36; tar_R_y=36; } else { tar_L_y=42; tar_R_y=42; }
}
void setEmotion_Idle() {
  tar_L_w=28; tar_L_h=30; tar_R_w=28; tar_R_h=30; tar_L_angle=0; tar_R_angle=0; tar_Bottom_cut=0;
  if (millis() - timerIdle > 4000) {
    if (random(0,10) > 4) { tar_L_h=2; tar_R_h=2; tar_L_y=34; tar_R_y=34; }
    else { int offset=random(-14,14); tar_L_x=24+offset; tar_R_x=74+offset; }
    timerIdle = millis();
  } else if (millis() - timerIdle > 400) { tar_L_h=30; tar_R_h=30; tar_L_y=16; tar_R_y=16; }
}
void setEmotion_Happy()     { tar_L_w=32; tar_L_h=34; tar_L_x=22; tar_L_y=12; tar_R_w=32; tar_R_h=34; tar_R_x=74; tar_R_y=12; tar_L_angle=0; tar_R_angle=0; tar_Bottom_cut=20; }
void setEmotion_Love()      { tar_L_w=8; tar_L_h=8; tar_L_x=36; tar_L_y=28; tar_R_w=8; tar_R_h=8; tar_R_x=86; tar_R_y=28; tar_Bottom_cut=0; tar_L_angle=0; tar_R_angle=0; }
void setEmotion_Music()     { tar_L_w=26; tar_L_h=22; tar_L_x=28; tar_R_w=26; tar_R_h=22; tar_R_x=74; tar_Bottom_cut=0; tar_L_angle=0; tar_R_angle=0; int bounce=sin(frameCount*0.5)*10; tar_L_y=20+bounce; tar_R_y=20+bounce; }
void setEmotion_Angry()     { tar_L_w=30; tar_L_h=20; tar_L_x=24; tar_L_y=26; tar_R_w=30; tar_R_h=20; tar_R_x=74; tar_R_y=26; tar_Bottom_cut=0; tar_L_angle=18; tar_R_angle=18; int rung=(frameCount%2==0)?3:-3; tar_L_x=24+rung; tar_R_x=74+rung; }
void setEmotion_Surprised() { tar_L_w=24; tar_L_h=38; tar_L_x=24; tar_L_y=8; tar_R_w=24; tar_R_h=38; tar_R_x=74; tar_R_y=8; tar_L_angle=0; tar_R_angle=0; tar_Bottom_cut=0; }
void setEmotion_Sad()       { tar_L_w=28; tar_L_h=24; tar_L_x=24; tar_L_y=24; tar_R_w=28; tar_R_h=24; tar_R_x=74; tar_R_y=24; tar_L_angle=-12; tar_R_angle=-12; tar_Bottom_cut=0; }
void setEmotion_Thinking()  { tar_L_w=28; tar_L_h=24; tar_L_x=24; tar_L_y=10; tar_R_w=28; tar_R_h=16; tar_R_x=74; tar_R_y=18; tar_L_angle=0; tar_R_angle=10; tar_Bottom_cut=0; }
void setEmotion_Bored()     { tar_L_w=30; tar_L_h=10; tar_L_x=24; tar_L_y=38; tar_R_w=30; tar_R_h=10; tar_R_x=74; tar_R_y=38; tar_L_angle=0; tar_R_angle=0; tar_Bottom_cut=0; }
void setEmotion_Wink()      { tar_L_w=30; tar_L_h=30; tar_L_x=24; tar_L_y=16; tar_R_w=30; tar_R_h=2; tar_R_x=74; tar_R_y=30; tar_L_angle=0; tar_R_angle=0; tar_Bottom_cut=0; }
void drawHeart(int x, int y, int size) { int s=size/2; display.fillCircle(x-s,y,s,WHITE); display.fillCircle(x+s,y,s,WHITE); display.fillTriangle(x-size,y+s/2,x+size,y+s/2,x,y+size*1.5,WHITE); }

void veMatVector() {
  display.clearDisplay();
  if (current_emotion == 9) {
    display.drawRect(42, 2, 44, 62, WHITE);
    if (active_game == 0) drawTetris(); else if (active_game == 1) drawSnake(); else drawPong();
  }
  else if (current_emotion == 10) {
    display.fillRoundRect(8, 0, 112, 18, 4, WHITE);
    display.fillTriangle(60, 18, 68, 18, 64, 24, WHITE);
    display.setTextColor(BLACK);
    display.setCursor(12, 5);
    int char_count = (millis() - timerTalk) / 80;
    String msg = getCauNoi(id_cau_noi);
    if (char_count > msg.length()) char_count = msg.length();
    display.print(msg.substring(0, char_count));
    display.setTextColor(WHITE);
  }
  else if (current_emotion == 4) {
    int beat = (frameCount % 10 < 5) ? 14 : 18; drawHeart(38,20,beat); drawHeart(88,20,beat); display.display(); return;
  }
  else if (current_emotion == 5) {
    display.drawRoundRect(14,6,100,36,12,WHITE); display.fillRect(14,24,100,20,BLACK);
    display.fillRoundRect(8,16,10,26,3,WHITE); display.fillRoundRect(110,16,10,26,3,WHITE);
    int nY = 18-(frameCount%12); display.fillCircle(64,nY,2,WHITE);
    display.drawLine(66,nY,66,nY-6,WHITE); display.drawLine(66,nY-6,70,nY-4,WHITE);
  }
  else if (current_emotion == 1) {
    int f=frameCount%90; display.setTextSize(1);
    if(f>0)  { display.setCursor(95,30-f/3); display.print("z"); }
    if(f>30) { display.setCursor(105,30-(f-30)/3); display.print("z"); }
    if(f>60) { display.setCursor(115,30-(f-60)/3); display.print("Z"); }
  }
  display.fillRoundRect(cur_L_x, cur_L_y, cur_L_w, cur_L_h, 6, WHITE);
  display.fillRoundRect(cur_R_x, cur_R_y, cur_R_w, cur_R_h, 6, WHITE);
  if (cur_L_angle > 1) display.fillTriangle(cur_L_x-5,cur_L_y-5,cur_L_x+cur_L_w+5,cur_L_y+cur_L_angle,cur_L_x+cur_L_w+5,cur_L_y-10,BLACK);
  else if (cur_L_angle < -1) display.fillTriangle(cur_L_x-5,cur_L_y-5,cur_L_x+cur_L_w+5,cur_L_y-5,cur_L_x-5,cur_L_y-cur_L_angle,BLACK);
  if (cur_R_angle > 1) display.fillTriangle(cur_R_x+cur_R_w+5,cur_R_y-5,cur_R_x-5,cur_R_y+cur_R_angle,cur_R_x-5,cur_R_y-10,BLACK);
  else if (cur_R_angle < -1) display.fillTriangle(cur_R_x+cur_R_w+5,cur_R_y-5,cur_R_x-5,cur_R_y-5,cur_R_x+cur_R_w+5,cur_R_y-cur_R_angle,BLACK);
  if (cur_Bottom_cut > 1) {
    display.fillCircle(cur_L_x+(cur_L_w/2),cur_L_y+cur_L_h,cur_Bottom_cut,BLACK);
    display.fillCircle(cur_R_x+(cur_R_w/2),cur_R_y+cur_R_h,cur_Bottom_cut,BLACK);
  }
  display.display();
}