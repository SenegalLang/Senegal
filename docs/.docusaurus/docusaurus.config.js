export default {
  "title": "Senegal",
  "tagline": "Senegal Programming Language",
  "url": "https://lang-senegal.web.app",
  "baseUrl": "/",
  "onBrokenLinks": "throw",
  "favicon": "img/favicon.ico",
  "themes": [
    "@docusaurus/theme-live-codeblock"
  ],
  "plugins": [
    "D:\\Projects\\Senegal\\docs\\node_modules\\@cmfcmf\\docusaurus-search-local\\src\\index.js",
    [
      "@docusaurus/plugin-content-docs",
      {
        "id": "community",
        "path": "community",
        "editUrl": "https://github.com/Calamity210/Senegal/edit/master/docs/",
        "routeBasePath": "community",
        "sidebarPath": "D:\\Projects\\Senegal\\docs\\sidebarsCommunity.js",
        "showLastUpdateAuthor": true,
        "showLastUpdateTime": true
      }
    ]
  ],
  "organizationName": "Calamity210",
  "projectName": "Senegal",
  "themeConfig": {
    "colorMode": {
      "defaultMode": "dark",
      "disableSwitch": false,
      "respectPrefersColorScheme": true,
      "switchConfig": {
        "darkIcon": "🌜",
        "darkIconStyle": {},
        "lightIcon": "🌞",
        "lightIconStyle": {}
      }
    },
    "announcementBar": {
      "id": "leave_star",
      "content": "⭐️ If you like our projects, give it a star on <a target=\"_blank\" rel=\"noopener noreferrer\" href=\"https://github.com/Calamity210/Senegal\">GitHub</a>! ⭐️",
      "backgroundColor": "#fff",
      "textColor": "#000",
      "isCloseable": true
    },
    "navbar": {
      "hideOnScroll": true,
      "title": "Senegal",
      "logo": {
        "alt": "Senegal Logo",
        "src": "img/logo.svg"
      },
      "items": [
        {
          "to": "docs/",
          "activeBasePath": "docs",
          "label": "Docs",
          "position": "left"
        },
        {
          "to": "community/support",
          "activeBasePath": "community",
          "label": "Support",
          "position": "left"
        },
        {
          "href": "https://github.com/Calamity210/Senegal",
          "label": "GitHub",
          "position": "right"
        }
      ]
    },
    "footer": {
      "style": "dark",
      "links": [
        {
          "title": "Community",
          "items": [
            {
              "label": "Discord",
              "href": "https://discord.gg/9dq6YB2"
            }
          ]
        },
        {
          "title": "More",
          "items": [
            {
              "label": "GitHub",
              "href": "https://github.com/Calamity210/Senegal"
            }
          ]
        }
      ],
      "copyright": "Copyright © 2020 SenegalLang"
    },
    "prism": {
      "additionalLanguages": [
        "dart"
      ],
      "defaultLanguage": "dart"
    }
  },
  "presets": [
    [
      "@docusaurus/preset-classic",
      {
        "docs": {
          "path": "docs",
          "sidebarPath": "D:\\Projects\\Senegal\\docs\\sidebars.js",
          "editUrl": "https://github.com/Calamity210/Senegal/edit/master/docs/",
          "showLastUpdateTime": true,
          "disableVersioning": false,
          "lastVersion": "current",
          "onlyIncludeVersions": [
            "current"
          ],
          "versions": {
            "current": {
              "label": "Next ('deploy preview')"
            }
          }
        },
        "theme": {
          "customCss": "D:\\Projects\\Senegal\\docs\\src\\css\\custom.css"
        }
      }
    ]
  ],
  "onDuplicateRoutes": "warn",
  "customFields": {}
};