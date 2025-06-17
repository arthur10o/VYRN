/project-root
│
├── frontend/
│   ├── index.html           # Page principale de l'IDE
│   ├── styles.css           # Styles CSS de l'éditeur
│   ├── editor.js            # Gestion de l'éditeur : récupération du code, coloration syntaxique
│
├── backend/
│   ├── server.py            # Mini serveur HTTP Python (reçoit JSON, transmet au C++)
│   ├── parser/              # Parser et génération code C++
│   │   ├── ast_parser.cpp   # Parseur AST en C++
│   │   ├── code_generator.cpp # Génération/interprétation du code
│   │   ├── CMakeLists.txt   # Pour compiler les fichiers C++
│
├── communication/
│   ├── protocol.md          # Documentation JSON envoyé du frontend vers serveur (format, clés)
│
├── README.md
└── Makefile / build scripts # Pour builder le backend C++
