import {themes as prismThemes} from 'prism-react-renderer';
import type {Config} from '@docusaurus/types';
import type * as Preset from '@docusaurus/preset-classic';
import type * as Mermaid from '@docusaurus/theme-mermaid';

//Backlog: https://github.com/users/StephaneCouturier/projects/7/views/1
//Repository: https://github.com/StephaneCouturier/Katalog

function suppressVscodeLspWarning() {
  return {
    name: 'suppress-vscode-languageserver-types-warning',
    configureWebpack() {
      return {
        ignoreWarnings: [
          { module: /vscode-languageserver-types/ },
        ],
      };
    },
  };
}

const config: Config = {
  title: 'Katalog',
  tagline: ' Manage catalogs of disks and files to search and get statistics ',
  favicon: 'img/favicon.ico',

  // Set the production url of your site here
  url: 'https://StephaneCouturier.github.io',
  // Set the /<baseUrl>/ pathname under which your site is served
  // For GitHub pages deployment, it is often '/<projectName>/'
  baseUrl: '/Katalog/',

  // GitHub pages deployment config.
  // If you aren't using GitHub pages, you don't need these.
  organizationName: 'StephaneCouturier', // Usually your GitHub org/user name.
  projectName: 'Katalog', // Usually your repo name.

  onBrokenLinks: 'throw',
  markdown: {
    mermaid: true,
    hooks: {
      onBrokenMarkdownLinks: 'warn',
    },
  },
  themes: ['@docusaurus/theme-mermaid'],
  plugins: [suppressVscodeLspWarning],

  // Even if you don't use internationalization, you can use this field to set
  // useful metadata like html lang. For example, if your site is Chinese, you
  // may want to replace "en" with "zh-Hans".
  i18n: {
    defaultLocale: 'en',
    locales: ['en', 'fr', 'cs'], //, 'de'
  },

  presets: [
    [
      'classic',
      {
        docs: {
          sidebarPath: './sidebars.ts',
          // Please change this to your repo.
          // Remove this to remove the "edit this page" links.
          //editUrl: 'https://github.com/facebook/docusaurus/tree/main/packages/create-docusaurus/templates/shared/',
        },
        blog: {
          showReadingTime: true,
          // Please change this to your repo.
          // Remove this to remove the "edit this page" links.
          // editUrl: 'https://github.com/facebook/docusaurus/tree/main/packages/create-docusaurus/templates/shared/',
        },
        theme: {
          customCss: './src/css/custom.css',
        },
      } satisfies Preset.Options,
    ],
  ],

  themeConfig: {
    // Replace with your project's social card
    image: 'img/Katalog_logo_1.20.png',
    navbar: {
      title: 'Katalog',
      logo: {
        alt: 'Katalog Site Logo',
        src: 'img/Katalog_logo_1.20.png',
      },
      items: [
        {
          type: 'docSidebar',
          sidebarId: 'tutorialSidebar',
          position: 'left',
          label: 'Documentation',
        },
        {
          to: '/blog',
          label: 'Blog',
          position: 'left'},
        {
          href: 'https://sourceforge.net/projects/katalogg/files/latest/download',
          label: 'Flatpak',
          position: 'right',
        },
        {
          href: 'https://sourceforge.net/projects/katalogg/files/latest/download',
          label: 'Download',
          position: 'right',
        },
        {
            href: 'https://github.com/StephaneCouturier/Katalog',
            label: 'GitHub',
            position: 'right',
        },
        {
          type: 'localeDropdown',
          position: 'right',
        },

      ],
    },
    footer: {
      style: 'dark',
      links: [
        {
          title: 'Documentation',
          items: [
            {
              label: 'Tutorial',
              to: '/docs/tutorial',
            },
          ],
        },
        {
          title: 'Community',
          items: [
            {
              label: 'Blog',
              to: '/blog',
            },
            {
              label: 'GitHub',
              href: 'https://github.com/StephaneCouturier/Katalog/discussions',
            },
            {
              label: 'Facebook',
              href: 'https://www.facebook.com/100068644945574/',
            },
          ],
        },
        {
          title: 'More',
          items: [
            {
              label: 'GitHub Repository',
              href: 'https://github.com/StephaneCouturier/Katalog',
            },
            {
              label: 'GitHub Backlog',
              href: 'https://github.com/users/StephaneCouturier/projects/7/views/1',
            },
          ],
        },
      ],
      copyright: `Copyright © 2026 Katalog`,
    },
    prism: {
      theme: prismThemes.github,
      darkTheme: prismThemes.dracula,
    },
    mermaid: {
      // Use Katalog color palette: deep blue, sky blue, lime green, orange
      theme: { light: 'base', dark: 'base' },
      options: {
        themeVariables: {
          // Root node
          cScale0: '#095676',
          // Branch 1 — File Management & Search
          cScale1: '#39b2e5',
          // Branch 2 — UI & Multi-platform
          cScale2: '#81d41a',
          // Branch 3 — Quality & Scale
          cScale3: '#ff8000',
        },
      },
    },
  } satisfies Preset.ThemeConfig & Mermaid.ThemeConfig,
};

export default config;
