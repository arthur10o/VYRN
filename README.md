# VYRN — Langage de programmation expérimental

---

## Table des matières

- [Présentation](#présentation)  
- [Fonctionnalités](#fonctionnalités-principales)
- [Installation](#installation)  
- [Utilisation](#utilisation)  
- [Langage VYRN — Documentation](#langage-vyrn--documentation)  
- [Architecture technique](#architecture-technique)  
- [Contribuer](#contribuer)  
- [Roadmap](#roadmap)  
- [Licence](#licence)  

---

## Présentation

**VYRN** est un langage de programmation expérimental, accompagné d’un environnement web léger permettant d’écrire et d’exécuter du code en temps réel.  
Le projet combine un IDE web (HTML/CSS/JS), un serveur Python pour interpréter les requêtes, et un exécutable C++ pour traiter les instructions du langage.

Le but est d’explorer la création d’un langage et de son environnement complet, avec un contrôle total et sans bibliothèques externes complexes.

---

## Fonctionnalités principales

- **Éditeur Web Intégré** :  Interface web simple pour écrire, envoyer et afficher les résultats du code.
- **Langage VYRN** : Syntaxe minimaliste avec support des types bool, string, float et int, et commandes simples (`print` notamment).  
- **Backend Python** : Serveur HTTP basé sur ```http.server``` qui parse le code, génère du C++ puis compile et exécute ce dernier.
- **Interpréteur C++** : Le serveur écrit un fichier ```.cpp``` temporaire, compile avec ```g++``` et exécute le binaire pour récupérer la sortie.
- **Communication via requêtes HTTP** (GET pour l’interface, POST pour exécuter le code).
- **Pas d’arrêt automatique du serveur**, pour un usage continu.

---

## Installation

### Prérequis

- Python 3.x  
- Navigateur web moderne (Chrome, Firefox, Edge)  
- (Optionnel) Compilateur C++ (`g++`) pour développer l'interpréteur  
- Assurez-vous que `g++` est dans votre PATH (MinGW pour Windows ou GCC sous Linux/macOS).

### Étapes

1. **Cloner le dépôt** ou télécharger les fichiers sources :

   ```bash
   git clone https://github.com/arthur10o/VYRN.git
   cd VYRN
2. **Lancer le serveur Python :**
   ```bash
   python3 app.py
3. Ouvrir l’IDE dans le navigateur :
   ```bash
   http://localhost:5500

---

## Utilisation
1. Ecrire du code VYRN dans l'éditeur
2. Cliquer sur **Exécuter** pour envoyer le code au serveur.
3. Visualiser la sortie ou les erreurs dans la console de résultats intégrée.
4. Le serveur **reste actif** après exécution.

---

## Langage VYRN — Documentation
### Syntaxe de base
| Commande             | Description                               | Exemple                   | Sortie           |
| -------------------- | ----------------------------------------- | ------------------------- | ---------------- |
| `print("texte");`    | Affiche une chaîne de caractères          | `print("Hello, World!");` | `Hello, World!`  |
| `print(variable);`   | Affiche la valeur d’une variable          | `print(age);`             | `30`             |
| `let nom = valeur;`  | Déclare une variable typée                | `let age = 30;`           | Variable stockée |
| `nom = valeur_deux;` | Redéclaration d'une variable de même type | `age = 32;`               | Variable stockée |

- Types supportés : ```string``` (exte entre guillemets doubles ou simples), ```bool``` (```true```/```false```), ```int``` (entiers), ```float``` (nombres à virgule).
- Les commentaires ```//``` sont supportés en début de ligne.
- Toute commande inconnue ou mal formée retourne une erreur.

### Exemples
```vy
// Déclaration de variables
let isHappy = true;
let name = "Alice";
let pi = 3.14;
let age = 30;

// Affichage de chaînes de caractères et de variables
print("Nom :");
print(name);
print("Âge :");
print(age);
print("Pi vaut :");
print(pi);
print("Heureux ?");
print(isHappy);

// Modification de variables
name = Tom;
pi = 3.141592;
age = 20;
isHappy = false;

// Afficher des nouvelles variables après modification
print("Nom :");
print(name);
print("Âge :");
print(age);
print("Pi vaut :");
print(pi);
print("Heureux ?");
print(isHappy);
```

### Limitations actuelles
- Commandes supportées : `print`, déclaration et assignation de variables via `let` et `=`.
- Types supportés : `bool`, `string`, `int`, `float`.
- Pas encore de structures de contrôle (conditions, boucles).
- Variables doivent être typées implicitement selon la valeur assignée à la déclaration.
- Pas de gestion d’erreurs avancée autre que syntaxique et type.

---


## Architecture technique
```mermaid
graph LR 
    A[Frontend HTML/CSS/JS] -->|POST JSON code| B[Serveur Python HTTP]
    B -->|Parse et génère C++| C[Code C++ temporaire]
    C -->|Compilation g++| D[Exécutable natif]
    D -->|Sortie texte| B
    B -->|Réponse texte| A
```

### Description des composants
- Frontend (HTML/CSS/JS):
    - Éditeur web en HTML/CSS/JS, envoie le code en POST JSON.
    - Affiche la sortie reçue du serveur.
- Serveur Python :
    - Service HTTP simple (```http.server```)
    - Parse le code VYRN, génère du code C++ correspondant.
    - Compile le C++ avec ```g++``` en exécutable temporaire.
    - Exécute le binaire et récupère la sortie pour la renvoyer.
- Compilation & Exécution :
    - Création de fichiers temporaires ```temp.cpp``` et ```temp_exec.exe``` (Windows).
    - Nettoyage automatique des fichiers temporaires après exécution.

---

## Contribuer
Contributions bienvenues !
Merci de :
- Ouvrir une issue pour discuter des idées ou bugs
- Proposer des pull requests claires avec tests et documentation
- Respecter la structure et la syntaxe existantes
- Ajouter des fonctionnalités progressivement

---

## Roadmap
- [x] IDE web simple (HTML/CSS/JS)
- [x] Serveur Python minimal
- [x] Parsing basique du langage VYRN
- [x] Génération et compilation dynamique du C++
- [x] Gestion des variables typées et ```print```
- [ ] Ajout de structures de contrôle (conditions, boucles)
- [ ] Amélioration de la gestion des erreurs
- [ ] Support des fonctions et modules
- [ ] Tests automatisés et documentation utilisateur complète

---

## Licence

Ce projet est distribué sous les termes de la **GNU General Public License v3.0 (GPL-3.0)**.  
Cela signifie que vous êtes libre de :

- **Utiliser** ce logiciel pour tout usage,
- **Étudier** et **modifier** le code source,
- **Partager** des copies du logiciel,
- **Distribuer** vos propres versions, à condition qu'elles soient également sous licence GPL-3.0.

Toute modification ou redistribution du projet doit rester sous la même licence, et le **code source doit être fourni** ou rendu accessible.

Consultez le fichier [`LICENSE`](LICENSE) pour le texte complet de la licence.
Pour plus d'informations, vous pouvez aussi visiter :  

---

Merci d’avoir testé VYRN !
