# 🤖 VECTOR FACE V8.2 - PET ROBOT EDITION

<p align="center">
  <a href="#-english">English</a> • 
  <a href="#-tiếng-việt">Tiếng Việt</a> • 
  <a href="#-한국어">한국어</a>
</p>

---

## 🇺🇸 ENGLISH

A smart, expressive, and autonomous companion pet robot powered by **Arduino Uno R3**. This project integrates a dual-motor drive system, parallel artificial intelligence for locomotion, cliff/obstacle avoidance, and a dynamic LERP animation engine to render real-time facial expressions and playable mini-games on an OLED display.

### 🛠️ Hardware Architecture
The system is engineered using the following core hardware components:
* **Microcontroller:** Arduino Uno R3.
* **Display:** 0.96-inch SSD1306 OLED Screen (128x64 resolution, I2C interface running at 400kHz).
* **Motor Driver:** L298N Dual H-Bridge Motor Controller.
* **Actuators:** 2x DC Gear Motors (Left & Right Wheels).
* **Sensors:** * `1x HC-SR04` Ultrasonic Sensor facing **forward** (Obstacle detection & human interaction).
  * `1x HC-SR04` Ultrasonic Sensor facing **downward** (Cliff/edge detection).

### 🧠 Key Features & AI State Machine
The robot operates as an autonomous agent using an independent, real-time Finite State Machine (FSM) running concurrently with the facial rendering pipeline:

1. **STATE_ROAM:** The robot explores its environment randomly, alternating between moving forward, turning left/right, slowing down, or pausing curiously every 2 seconds.
2. **STATE_AVOID:** When the forward HC-SR04 detects an obstacle within $\le 20\text{cm}$, the robot automatically triggers a backing-up sequence, changes its facial expression to *Angry* or *Surprised*, and makes a tactical turn to escape.
3. **STATE_CLIFF (Highest Priority):** If the floor-facing sensor reads a distance $> 12\text{cm}$ (indicating an edge, table cliff, or stairs), the robot instantly goes into an emergency protocol: stops, expresses panic (*Surprised*), reverses rapidly, and pivots away.
4. **STATE_INTERACT:** When a human approach is detected between $20\text{cm}$ and $50\text{cm}$, the robot halts immediately to "stare" at the user, shifting through multiple mood cycles (*Happy, Thinking, Wink, Bored*).
5. **Multi-Game Idle Engine:** If a user gets extremely close ($< 6\text{cm}$), the robot activates its internal gaming engine, running standalone retro titles (**Tetris, Snake, or Pong**) on its screen where its eyes "follow" the game objects.
6. **Alone & Social Speech System:** When left alone, the robot goes to sleep (rendering "zZZ" animations). Eventually, it wakes up randomly to talk to itself by typing out friendly text bubbles with matching emotions.

### 📌 Pin Mapping

#### 1. Ultrasonic Sensors (HC-SR04)
* **TRIG_PIN** (Forward): `D2`
* **ECHO_PIN** (Forward): `D3`
* **TRIG_FLOOR** (Cliff): `D4`
* **ECHO_FLOOR** (Cliff): `D5`

#### 2. L298N Motor Driver
* **IN1** / **IN2**: `D6` / `D7` (Left Motor Direction)
* **ENA**: `D9` (Left Motor Speed - PWM)
* **IN3** / **IN4**: `D10` / `D11` (Right Motor Direction)
* **ENB**: `D8` (Right Motor Enable)

#### 3. SSD1306 OLED Display (0.96")
* **SDA**: `A4` | **SCL**: `A5` (I2C Interface)

---

## 🇻🇳 TIẾNG VIỆT

Dự án chế tạo robot thú cưng thông minh, biết biểu cảm và tự hành sử dụng vi điều khiển **Arduino Uno R3**. Hệ thống tích hợp mạch điều khiển động cơ kép, trí tuệ nhân tạo độc lập cho việc di chuyển di động, né vật cản/vực thẳm, kết hợp cùng bộ render chuyển động LERP mượt mà giúp hiển thị các sắc thái khuôn mặt và các trò chơi mini trực quan trên màn hình OLED.

### 🛠️ Thành Phần Phần Cứng
Hệ thống được cấu thành từ các thiết bị phần cứng cốt lõi sau:
* **Vi điều khiển chính:** Arduino Uno R3.
* **Màn hình hiển thị:** OLED SSD1306 0.96 inch (Độ phân giải 128x64, chuẩn giao tiếp I2C tốc độ cao 400kHz).
* **Mạch điều khiển động cơ:** L298N Dual H-Bridge.
* **Động cơ:** 2x Động cơ DC giảm tốc (Bánh trái & Bánh phải).
* **Hệ thống cảm biến:**
  * `1x HC-SR04` Cảm biến siêu âm hướng **phía trước** (Phát hiện vật cản & nhận biết tương tác).
  * `1x HC-SR04` Cảm biến siêu âm hướng **xuống đất** (Phát hiện vực thẳm, mép bàn).

### 🧠 Cơ Chế Hoạt Động & Trạng Thái AI
Robot hoạt động như một thực thể tự hành thông qua thuật toán Phân trạng thái (Finite State Machine) chạy song song với luồng xử lý đồ họa:

1. **STATE_ROAM (Tự do đi chơi):** Robot tự khám phá môi trường ngẫu nhiên: đi thẳng, quẹo trái/phải, đi chậm hoặc đứng im ngơ ngác, tự động chuyển đổi hành động sau mỗi 2 giây.
2. **STATE_AVOID (Né vật cản phía trước):** Khi cảm biến trước phát hiện vật cản khoảng cách $\le 20\text{cm}$, robot tự động lùi lại, đổi biểu cảm sang *Angry (Giận dữ)* hoặc *Surprised (Hoảng sợ)* và quay góc ngẫu nhiên để tránh.
3. **STATE_CLIFF (Chống rơi vực - Ưu tiên cao nhất):** Khi cảm biến sàn đo được khoảng cách $> 12\text{cm}$ (hết đất, mép bàn, bậc thang), robot lập tức kích hoạt chế độ khẩn cấp: dừng bánh, biểu cảm hoảng hốt, lùi nhanh và xoay người sang hướng khác.
4. **STATE_INTERACT (Tương tác tò mò):** Khi phát hiện con người ở khoảng cách từ $20\text{cm}$ đến $50\text{cm}$, robot sẽ đứng yên để "ngắm nhìn" và liên tục đổi cảm xúc (*Happy, Thinking, Wink, Bored*).
5. **Hệ thống Game Mini:** Khi người dùng đưa tay lại cực gần ($< 6\text{cm}$), màn hình robot sẽ biến thành máy chơi game cổ điển tự động với các trò **Tetris, Snake, hoặc Pong**, mắt của robot sẽ di chuyển liếc nhìn theo vật thể trong game.
6. **Hệ thống nói chuyện & Đi ngủ:** Khi bị bỏ rơi quá lâu, robot sẽ tự ngủ (hiển thị hiệu ứng "zZZ"). Thỉnh thoảng, nó tự tỉnh giấc gõ bóng chat nói chuyện một mình với các câu thoại đáng yêu kèm cảm xúc tương ứng.

### 📌 Sơ Đồ Đấu Nối Chân (Pin Mapping)

#### 1. Cảm biến siêu âm (HC-SR04)
* **TRIG_PIN** (Phía trước): `D2`
* **ECHO_PIN** (Phía trước): `D3`
* **TRIG_FLOOR** (Né vực): `D4`
* **ECHO_FLOOR** (Né vực): `D5`

#### 2. Mạch điều khiển động cơ L298N
* **IN1** / **IN2**: `D6` / `D7` (Điều hướng động cơ Trái)
* **ENA**: `D9` (Điều khiển tốc độ động cơ Trái - PWM)
* **IN3** / **IN4**: `D10` / `D11` (Điều hướng động cơ Phải)
* **ENB**: `D8` (Kích hoạt động cơ Phải)

#### 3. Màn hình OLED SSD1306 (0.96")
* **SDA**: Chân `A4` | **SCL**: Chân `A5` (Giao tiếp I2C)

---

## 🇰🇷 한국어

**Arduino Uno R3**를 기반으로 구동되는 스마트하고 감정 표현이 가능한 자율 주행 반려 로봇 프로젝트입니다. 본 프로젝트는 듀얼 모터 드라이브 시스템, 자율 이동을 위한 병렬 인공지능 알고리즘, 장애물 및 낭떠러지 회피 기능, 그리고 OLED 디스플레이에 실시간 얼굴 표정과 미니 게임을 부드럽게 구현하는 동적 LERP 애니메이션 엔진을 통합했습니다.

### 🛠️ 하드웨어 구성
본 시스템은 다음과 같은 핵심 하드웨어 부품으로 설계되었습니다:
* **마이크로컨트롤러:** Arduino Uno R3.
* **디스플레이:** 0.96인치 SSD1306 OLED 스크린 (128x64 해상도, 400kHz 고속 I2C 통신).
* **모터 드라이버:** L298N 듀얼 H-브릿지 모터 컨트롤러.
* **액추에이터:** 2x DC 감속 모터 (좌측 및 우측 바퀴).
* **센서 군:**
  * `1x HC-SR04` 초음파 센서 **정면 방향** (장애물 감지 및 인간 상호작용).
  * `1x HC-SR04` 초음파 센서 **바닥 방향** (낭떠러지 및 테이블 가장자리 감지).

### 🧠 핵심 기능 및 AI 상태 머신
로봇은 그래픽 렌더링 파이프라인과 병렬로 실행되는 실시간 유한 상태 머신(FSM)을 통해 자율적으로 판단하고 행동합니다:

1. **STATE_ROAM (자유 탐색):** 로봇은 2초마다 직진, 좌/우회전, 감속, 호기심 어린 정지 상태를 무작위로 전환하며 주변 환경을 자율적으로 탐색합니다.
2. **STATE_AVOID (전방 장애물 회피):** 전방 초음파 센서가 $\le 20\text{cm}$ 이내에서 장애물을 감지하면, 로봇은 자동으로 후진 시퀀스를 트리거하고 표정을 *Angry(화남)* 또는 *Surprised(놀람)*로 바꾼 뒤 회피 회전을 수행합니다.
3. **STATE_CLIFF (낭떠러지 방지 - 최우선 순위):** 바닥을 향한 센서의 측정 거리가 $> 12\text{cm}$ 초과할 경우(테이블 가장자리, 계단 등), 로봇은 즉시 비상 프로토콜을 가동합니다: 바퀴를 멈추고 공포(*Surprised*) 표정을 지으며 신속하게 후진한 후 안전한 방향으로 회전합니다.
4. **STATE_INTERACT (인간 상호작용):** $20\text{cm}$에서 $50\text{cm}$ 사이에서 사람의 접근이 감지되면, 로봇은 즉시 멈춰 서서 사용자를 바라보며 다양한 감정 사이클(*Happy, Thinking, Wink, Bored*)을 무작위로 표현합니다.
5. **내장 미니 게임 엔진:** 사용자가 매우 가까이($< 6\text{cm}$) 접근하면 로봇의 화면이 클래식 레트로 게임 화면(**Tetris, Snake, 또는 Pong**)으로 전환되며, 로봇의 눈이 게임 오브젝트의 움직임을 실시간으로 쫓아갑니다.
6. **자가 말하기 및 수면 시스템:** 오랜 시간 동안 상호작용이 없으면 로봇은 수면 모드("zZZ" 애니메이션)로 진입합니다. 주기적으로 스스로 깨어나 감정에 맞는 귀여운 대사 말풍선을 띄우며 혼자서 상호작용을 시도합니다.

### 📌 핀 맵 구성 (Pin Mapping)

#### 1. 초음파 센서 (HC-SR04)
* **TRIG_PIN** (전방 센서): `D2`
* **ECHO_PIN** (전방 센서): `D3`
* **TRIG_FLOOR** (바닥 센서): `D4`
* **ECHO_FLOOR** (바닥 센서): `D5`

#### 2. L298N 모터 드라이버
* **IN1** / **IN2**: `D6` / `D7` (좌측 모터 방향 제어)
* **ENA**: `D9` (좌측 모터 속도 제어 - PWM)
* **IN3** / **IN4**: `D10` / `D11` (우측 모터 방향 제어)
* **ENB**: `D8` (우측 모터 활성화)

#### 3. SSD1306 OLED 디스플레이 (0.96")
* **SDA**: `A4` | **SCL**: `A5` (I2C 인터페이스)

---

## 📬 Contact / Contacts / 연락처
* **Developer:** Nguyễn Đức Dũng (덕용)
* **Email:** nguyendungduc985@gmail.com
* **Major:** Artificial Intelligence (AI) @ East Asia University (Đại học Đông Á)
