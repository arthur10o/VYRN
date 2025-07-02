/project-root
│
├── frontend/
│   ├── index.html                # Page principale de l'IDE
│   ├── style.css                 # Styles CSS de l'éditeur
│   ├── main.js                   # Gestion de l'éditeur : récupération du code, coloration syntaxique
│
├── backend/
│   ├── server.py                 # Mini serveur HTTP Python (reçoit JSON, transmet au C++)
│   ├── parser/                   # Parser et génération code C++
│   │   ├── ast_parser.hpp        # Définition AST et parseur
│   │   ├── code_generator.cpp    # Génération/interprétation du code
│   │   ├── parser_exec.exe       # Le fichier exécutable généré par le C++
│
├── communication/
│   ├── input_code.txt            # Code VYRN reçu du frontend
│   ├── generated_code.cpp        # Code C++ généré
│   ├── program_output.txt        # Sortie du programme exécuté
│   ├── compile_errors.txt        # Erreurs de compilation éventuelles
│   ├── parsing_errors.txt        # Erreurs de parsing éventuelles
│
├── LICENSE
└── README.md