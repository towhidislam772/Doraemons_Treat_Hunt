# 🎮 Doraemon's Treat Hunt

<div align="center">

![OpenGL](https://img.shields.io/badge/OpenGL-2D-blue)
![C++](https://img.shields.io/badge/C++-11-green)
![Platform](https://img.shields.io/badge/Platform-Windows-lightgrey)
![License](https://img.shields.io/badge/License-Educational-orange)

**A 2D Interactive Side-Scrolling Adventure Game**

*Built with C++, OpenGL, and GLUT for Computer Graphics Course*

[Features](#-features) • [Installation](#-installation) • [How to Play](#-how-to-play) • [Screenshots](#-screenshots) • [Documentation](#-documentation)

</div>

---

## 📖 About The Project

**Doraemon's Treat Hunt** is a fully-featured 2D platformer game developed using OpenGL and GLUT as part of a Computer Graphics course project. The game features a Doraemon-inspired character navigating through **5 unique levels**, collecting doracakes while avoiding various obstacles and hazards.

### 🎯 Project Objectives
- Demonstrate practical implementation of Computer Graphics concepts
- Create an interactive, real-time graphics application
- Implement game mechanics: collision detection, animation, user input handling
- Showcase visual design with OpenGL primitives (no external sprites)

---

## ✨ Features

### 🎮 Gameplay Features
- **5 Unique Levels** with distinct visual themes and difficulty progression
- **Side-scrolling gameplay** with smooth character movement and jumping
- **Collectibles System**: Gather 10 doracakes per level
- **Lives System**: 3 hearts representing player health
- **Dynamic Obstacles**: Randomized placement and movement patterns
- **Level-Specific Hazards**:
  - Level 1: Basic moving obstacles
  - Level 2: Evening environment with fires
  - Level 3: Night scene with stones
  - Level 4: Falling rocks in rainy dawn
  - Level 5: Ghost enemies and lightning storms

### 🎨 Visual Features
- **Hand-drawn graphics** using OpenGL primitives (GL_POLYGON, GL_TRIANGLE_FAN, etc.)
- **Animated elements**: Character leg movement, clouds, environmental effects
- **Progressive visual themes**: Day → Evening → Night → Rainy Dawn → Stormy Night
- **Special effects**: Invincibility flash, hit feedback, lightning strikes

### 🎵 Audio Features
- **Background music** with custom soundtrack support via `music_config.txt`
- **Sound effects** for:
  - Doracake collection
  - Obstacle collision
  - Level completion
  - Game over
  - Final victory

### 💾 Game Systems
- **Per-level high score** saving in `highscore.txt`
- **Persistent storage** across game sessions
- **Pause functionality** (Press `P` key)
- **Score tracking**: Points earned from collecting doracakes
- **Invincibility frames** after taking damage

---

## 🛠️ Technologies Used

| Component | Technology |
|-----------|-----------|
| **Language** | C++ (C++11 standard) |
| **Graphics API** | OpenGL 2.1 |
| **Windowing & Input** | GLUT (FreeGLUT) |
| **Audio System** | Windows Multimedia (winmm.lib, mciSendString) |
| **IDE** | Code::Blocks with MinGW/GCC |
| **Platform** | Windows (primary), with cross-platform stubs for Linux/Mac |

---

## 📋 Requirements

### System Requirements
- **OS**: Windows 7/8/10/11 (or Linux/Mac with modifications)
- **Compiler**: GCC/MinGW with C++11 support
- **RAM**: 512 MB minimum
- **Graphics**: OpenGL 2.1 compatible GPU

### Development Requirements
- Code::Blocks IDE (or any C++ IDE)
- OpenGL libraries (opengl32, glu32)
- GLUT library (freeglut)
- Windows Multimedia library (winmm) - for audio on Windows

---

## 🚀 Installation

### Step 1: Clone the Repository
```bash
git clone https://github.com/towhidislam772/Doraemons_Treat_Hunt.git
cd Doraemons_Treat_Hunt
```

### Step 2: Set Up Sound Files
Create a `sounds` folder next to your executable and add these files:
```
sounds/
├── bgm.mp3          # Background music (looping)
├── collect.wav      # Doracake collection sound
├── hit.wav          # Obstacle hit sound
├── levelup.wav      # Level completion sound
├── gameover.wav     # Game over sound
└── gamewin.wav      # Final victory sound
```

**Note**: You can customize the background music by creating a `music_config.txt` file:
```
sounds/my_custom_music.mp3
```

### Step 3: Compile the Project

#### Using Code::Blocks:
1. Open the project file (`.cbp`) in Code::Blocks
2. Go to **Project → Build Options → Linker Settings**
3. Add the following libraries:
   - `opengl32`
   - `glu32`
   - `freeglut`
   - `winmm` (for Windows audio)
4. Build and run (F9)

#### Using Command Line (MinGW):
```bash
g++ -o doraemon_hunt main.cpp -lopengl32 -lglu32 -lfreeglut -lwinmm -std=c++11
```

### Step 4: Run the Game
```bash
./doraemon_hunt.exe
```

---

## 🎮 How to Play

### Controls
| Key | Action |
|-----|--------|
| **Right Arrow** | Move right |
| **Left Arrow** | Move left |
| **Up Arrow** | Jump |
| **P** | Pause/Resume game |
| **Mouse Click** | Select levels and menu buttons |

### Objective
- **Collect 10 doracakes** in each level to advance
- **Avoid obstacles** to preserve your 3 lives
- **Achieve high scores** by collecting doracakes quickly
- **Complete all 5 levels** to win the game

### Gameplay Mechanics
1. **Movement**: Use arrow keys to navigate the side-scrolling world
2. **Jumping**: Press Up arrow to jump over obstacles
3. **Collection**: Touch doracakes to collect them (+10 points each)
4. **Damage**: Touching obstacles removes 1 heart (life)
5. **Invincibility**: Brief invincibility period after taking damage
6. **Level Progression**: Collect all doracakes to unlock "Next Level" button

### Level Themes
1. **Level 1 - Bright Day**: Green grass, blue sky, animated clouds
2. **Level 2 - Evening Sunset**: Orange/purple gradient sky, fires
3. **Level 3 - Dark Night**: Dark blue background, stone obstacles
4. **Level 4 - Rainy Dawn**: Falling rocks, rain effects
5. **Level 5 - Storm Night**: Ghost enemies, lightning strikes, twinkling stars

---

## 📸 Screenshots

### Home Screen
*Main menu with level selection buttons*

### Level 1 - Day Environment
*Bright, cheerful atmosphere with clouds and basic obstacles*

### Level 3 - Night Environment
*Dark scene with stone hazards*

### Level 5 - Storm Night
*Challenging final level with ghost and lightning*

---

## 🏗️ Project Structure

```
Doraemons_Treat_Hunt/
├── main.cpp                    # Main game source code
├── sounds/                     # Audio files directory
│   ├── bgm.mp3
│   ├── collect.wav
│   ├── hit.wav
│   ├── levelup.wav
│   ├── gameover.wav
│   └── gamewin.wav
├── highscore.txt              # Per-level high scores (auto-generated)
├── music_config.txt           # Custom music path (optional)
└── README.md                  # This file
```

---

## 🎓 Academic Information

### Course Details
- **Course**: Computer Graphics
- **Platform**: C++, OpenGL, GLUT, Code::Blocks
- **Submission Date**: May 15, 2026

### Project Team
| Name | Student ID | Role |
|------|-----------|------|
| **Towhidul Islam** | 23-55036-3 | Developer |
| Shafayat Jamil | 23-55457-7 | Developer |
| Abrar Kabir | 23-55095-3 | Developer |
| Aditya Roy | 23-55077-3 | Developer |

**Instructor**: Dipta Justin Gomes

---

## 🔧 Key Implementation Details

### Graphics Rendering
- All visuals drawn using OpenGL primitives (no texture mapping)
- Character constructed from circles, rectangles, and polygons
- Coordinate system: `gluOrtho2D(-9.50, 9.50, -10.0, 10.0)`

### Game Loop
- **Update frequency**: 60 FPS (16ms timer)
- **Display callback**: Renders current game state
- **Timer callback**: Updates positions, animations, collisions

### Collision Detection
- **Collectibles**: Distance-based (radius check)
- **Obstacles**: Bounding box intersection
- **Invincibility frames**: 45 frames (~0.75 seconds)

### File Management
- **High scores**: Plain text format in `highscore.txt`
- **Music config**: User-customizable soundtrack path
- **Auto-save**: Scores saved after each level completion

---

## 🔮 Future Enhancements

Potential improvements for future versions:

- [ ] **Texture Mapping**: Replace primitives with sprite images
- [ ] **Cross-Platform Audio**: Support for Linux/Mac audio systems
- [ ] **Level Editor**: External file-based level design
- [ ] **Player Profiles**: Multiple user accounts with separate scores
- [ ] **Enhanced Physics**: Delta-time independent movement
- [ ] **Enemy AI**: Advanced patrolling and chasing behaviors
- [ ] **Settings Menu**: Volume control, difficulty selection
- [ ] **Mobile Port**: Touch controls for Android/iOS
- [ ] **Multiplayer**: Local co-op or competitive modes
- [ ] **Achievements System**: Unlock rewards for special challenges

---

## 🐛 Known Limitations

1. **Windows-only audio**: Sound system uses Windows multimedia API
2. **Hard-coded levels**: Level data embedded in source code
3. **Primitive graphics**: No texture/sprite support (educational constraint)
4. **Simplified collision**: Basic hitbox detection only
5. **Fixed resolution**: 800x600 window size

---

## 📚 Documentation

### Key Functions

| Function | Purpose |
|----------|---------|
| `drawDoraemon()` | Renders character with primitives |
| `Level1()` to `Level5()` | Draw and update each level |
| `checkCollectibles()` | Detect doracake collection |
| `checkObstacles()` | Handle obstacle collisions |
| `updateObstacles()` | Animate obstacle movement |
| `playBGM()` | Start background music loop |
| `saveHiScore()` | Write scores to file |
| `loadHiScore()` | Read saved scores |

### Architecture
The game follows an event-driven architecture:
1. **Initialization** (`init()`) → Set up OpenGL state
2. **Main Loop** (`glutMainLoop()`) → Process events
3. **Display Callback** (`display()`) → Render frame
4. **Timer Callback** (`update()`) → Game logic
5. **Input Handlers** (`keyPress()`, `mouseClick()`) → User interaction

---

## 🤝 Contributing

This is an academic project, but contributions are welcome for educational purposes:

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

---

## 📄 License

This project is created for educational purposes as part of a Computer Graphics course. Feel free to use it for learning and academic purposes with proper attribution.

---

## 🙏 Acknowledgments

- **OpenGL Community** for comprehensive documentation
- **FreeGLUT** for cross-platform windowing
- **Course Instructor** for guidance and support
- **Doraemon** franchise for character inspiration (fan project, non-commercial)

---

## 📞 Contact

**Towhidul Islam**  
Student ID: 23-55036-3  
Email: towhidislam3624@gmail.com  
GitHub: [@towhidislam772](https://github.com/towhidislam772)

**Project Link**: [https://github.com/towhidislam772/Doraemons_Treat_Hunt](https://github.com/towhidislam772/Doraemons_Treat_Hunt)

---

<div align="center">

**⭐ If you found this project helpful, please give it a star! ⭐**

Made with ❤️ using OpenGL and C++

</div>
