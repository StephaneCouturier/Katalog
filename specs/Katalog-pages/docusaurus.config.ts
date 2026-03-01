import {themes as prismThemes} from 'prism-react-renderer';
import type {Config} from '@docusaurus/types';
import type * as Preset from '@docusaurus/preset-classic';

//Backlog: https://github.com/users/StephaneCouturier/projects/7/views/1
//Repository: https://github.com/StephaneCouturier/Katalog

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
    hooks: {
      onBrokenMarkdownLinks: 'warn',
    },
  },

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
      copyright: `Copyright © ${new Date().getFullYear()} Katalog (Pages built with Docusaurus)`,
    },
    prism: {
      theme: prismThemes.github,
      darkTheme: prismThemes.dracula,
    },
  } satisfies Preset.ThemeConfig,
};

export default config;
