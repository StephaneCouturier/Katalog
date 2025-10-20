# Aperçu général
## Indexation de fichiers & Gestionnaire de périphériques de stockage
Katalog est un puissant <b>gestionnaire de catalogue de fichiers</b> qui vous aide à organiser et à retrouver vos fichiers.<br/>
Il donne une vue complète des fichiers présents sur <b>plusieurs périphériques de stockage sans avoir besoin de les connecter</b>.<br/>
Katalog crée des <b>index complets pour que vous puissiez rechercher dans toute votre collection de fichiers depuis un seul endroit</b>, que vous gériez des disques internes et externes, des clés USB, des stockages réseau ou des disques optiques (Blu-ray, DVD, CD).

- **Cataloguez tout** : Créez des index détaillés des fichiers de n'importe quel périphérique de stockage
- **Recherchez hors ligne** : Trouvez des fichiers instantanément sans connecter ou monter le périphérique d'origine
- **Organisez votre collection** : Gérez plusieurs périphériques de stockage et leurs catalogues dans une hiérarchie unifiée


![](/img/screen_search_01.png)
<b>8 onglets principaux pour 8 fonctionnalités principales</b>
![](/img/global_tabwidget.png)
1. [Créer](Create) des catalogues de **fichiers**
1. [Rechercher](Search) des fichiers sur plusieurs périphériques de stockage **sans avoir besoin de les connecter**
1. [Explorer](Explore) la hiérarchie des catalogues et les fichiers
1. Organiser les [Périphériques](Devices) de stockage et leurs catalogues dans une hiérarchie unifiée avec des périphériques virtuels
1. Obtenir des [Statistiques](Statistics) sur vos collections de fichiers et l'utilisation du stockage
1. Personnaliser les [Étiquettes](Tags) et les attribuer aux répertoires pour des recherches ou statistiques supplémentaires
1. Comparer les catalogues de [Sauvegarde](BackUp) pour confirmer la couverture des fichiers et dossiers sauvegardés entre les périphériques source et cible
1. Personnaliser votre expérience avec vos [Paramètres](Settings) tels que la langue et le thème
<br/>

<div className="row">
  <div className="col col--6">
  <br/><br/>et un <b>panneau de [Sélection](Selection)</b> pour
  <br/>
  * Choisir de rechercher dans les <b>Catalogues</b> ou directement dans les <b>Lecteurs connectés</b><br/>
  * Définir le périphérique dans la <b>hiérarchie à utiliser</b> pour Rechercher ou Créer ou obtenir des Statistiques ou gérer la couverture de Sauvegarde<br/>
  </div>
  <div className="col col--6" style={{maxWidth: '200px'}}>
    ![](/img/global_selection_panel.png)
  </div>
</div>

---
## Fonctionnalités principales

### Informations détaillées sur les fichiers
- **[Intelligence du type de fichier](Create#enhanced-file-type-filtering)** : Détection standard par extension et vérification du type MIME
- **[Options de catalogue](Create#select-options-to-includeexclude-files)** : Inclure uniquement un [type de fichiers](Create#enhanced-file-type-filtering), et [inclure/exclure les répertoires ou fichiers cachés](Create#other-options)
- **[Extraction de métadonnées](Create#metadata-extraction)** : Extraction automatique des métadonnées des images (dimensions, informations de l'appareil photo), vidéos (durée, résolution) et fichiers audio (artiste, album, durée) ou tout autre type de fichier.
- **[Système d'étiquettes de dossier](Tags)** : Organisez et catégorisez les dossiers avec des étiquettes personnalisées

### Recherche et découverte puissantes
- **[Paramètres de recherche avancés](Search#search-text-criteria)** : Trouvez des fichiers par nom, chemin, taille, date, type de fichier et métadonnées
- **[Filtrage intelligent](Search#file-criteria)** : Utilisez plusieurs critères simultanément pour affiner rapidement les résultats
- **[Trouver les doublons](Search#duplicates-on)** : Identifiez les fichiers en double sur différents périphériques de stockage
- **[Trouver les différences](Search#differences-on)** : Visualisez les différences entre deux emplacements de stockage ou versions de sauvegarde


### Gestion des périphériques et catalogues
- **[Organisation des périphériques](Devices)** : Organisez les périphériques de stockage dans une structure hiérarchique (Virtuel > Stockage > Catalogues)
- **[Mise à jour](DevicesCatalogs)** : Maintenez les catalogues à jour avec des mises à jour manuelles ou automatiques
- **[Support d'importation](DevicesCatalogs#import)** : Importez des catalogues d'autres outils comme VVV


### Analyse et gestion
- **[Explorateur de fichiers](Explore)** : Parcourez le contenu des catalogues comme si le périphérique était connecté
- **[Statistiques](Statistics)** : Suivez vos collections de fichiers et l'utilisation du stockage
- **[Gestion des sauvegardes](BackUp)** : Cartographiez et comparez les répertoires sources avec leurs sauvegardes
- **[Opérations par lots](Search#batch-process)** : Exportez les résultats et effectuez des actions par lots sur les fichiers

### Capacités avancées
- **[Interface en ligne de commande](CommandLines)** : Automatisez les mises à jour de catalogues et les recherches via la ligne de commande (Linux)
- **[Flexibilité de la base de données](Settings#collection)** : Choisissez entre le stockage en fichiers CSV ou base de données SQLite

---
## Support multi-plateformes
| Système d'exploitation principal | Distributions / Versions    | Packaging    |
|-------------------|-------------|-------------|
| <div style={{display: 'flex', alignItems: 'center', gap: '10px'}}><img src={require('/img/linux.png').default} width="40" /> GNU/Linux</div>         | Tout 64bits, glibc 2.38+ <br/>Tout 32bits, glibc 2.35     | AppImage <br/>Portable |
| <div style={{display: 'flex', alignItems: 'center', gap: '10px'}}><img src={require('/img/windows.png').default} width="40" /> Microsoft Windows</div> | 64bits : Windows 10 & Windows 11    | Installateur <br/> Portable       |
| <div style={{display: 'flex', alignItems: 'center', gap: '10px'}}><img src={require('/img/macos.png').default} width="40" /> Apple macOS</div>       | 14+      | Installateur<br/> Portable       |

---
## Support multilingue

### Langues de l'application
Katalog est disponible en :

| Locale   | Langue       |
|----------|--------------|
| bg_BG    | Bulgare      |
| cz_CZ    | Tchèque      |
| da_DK    | Danois       |
| de_DE    | Allemand     |
| en_US    | Anglais      |
| es_ES    | Espagnol     |
| et_EE    | Estonien     |
| fi_FI    | Finnois      |
| fr_FR    | Français     |
| el_GR    | Grec         |
| hi_IN    | Hindi        |
| hr_HR    | Croate       |
| hu_HU    | Hongrois     |
| id_ID    | Indonésien   |
| it_IT    | Italien      |
| ja_JP    | Japonais     |
| lt_LT    | Lituanien    |
| lv_LV    | Letton       |
| nb_NO    | Norvégien    |
| nl_NL    | Néerlandais  |
| pl_PL    | Polonais     |
| pt_PT    | Portugais    |
| ro_RO    | Roumain      |
| si_SI    | Slovène      |
| sk_SK    | Slovaque     |
| sr_RS    | Serbe        |
| sv_SE    | Suédois      |
| uk_UA    | Ukrainien    |
| zh_CN    | Chinois      |

### Langues de la documentation
La documentation est disponible en :
| Locale   | Langue    |
|----------|-----------|
| en_US    | Anglais   |
| cz_CZ    | Tchèque   |
| fr_FR    | Français  |

---

**Prêt à organiser votre collection de fichiers ?** Commencez avec le **[Tutoriel](tutorial)** pour créer votre premier catalogue en moins de 5 minutes.