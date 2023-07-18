// @ts-check
// Note: type annotations allow type checking and IDEs autocompletion

const lightCodeTheme = require('prism-react-renderer/themes/github');
const darkCodeTheme = require('prism-react-renderer/themes/dracula');

/** @type {import('@docusaurus/types').Config} */
const config = {
  title: 'STD DAQ DOC',
  tagline: 'Standard data acquisition documentation',
  url: 'https://controls-ci.gitpages.psi.ch/',
  baseUrl: '/std_detector_buffer/',
  onBrokenLinks: 'throw',
  onBrokenMarkdownLinks: 'warn',
  favicon: 'img/favicon.ico',
  organizationName: 'Paul Scherrer Institu', // Usually your GitHub org/user name.
  projectName: 'std_detector_buffer', // Usually your repo name.

  presets: [
    [
      'classic',
      /** @type {import('@docusaurus/preset-classic').Options} */
      ({
        docs: {
          sidebarPath: require.resolve('./sidebars.js'),
          showLastUpdateAuthor: true,
          showLastUpdateTime: true,
          sidebarCollapsible: true,
          remarkPlugins: [[require('@docusaurus/remark-plugin-npm2yarn'), { sync: true }]],
          editUrl: ({ docPath }) =>
            `'https://git.psi.ch/controls-ci/std_detector_buffer/-/edit/master/website/std-daq-doc/docs/${docPath}`,
        },
        theme: {
          customCss: require.resolve('./src/css/custom.css'),
        },
      }),
    ],
  ],
  plugins: [require.resolve('docusaurus-lunr-search')],

  themeConfig:
    /** @type {import('@docusaurus/preset-classic').ThemeConfig} */
    ({ 
      editUrl: ',
      navbar: {
        title: 'STD DAQ DOC',
        logo: {
          alt: 'std-daq-doc',
          src: 'img/logo.svg',
        },
        items: [
          {
            type: 'doc',
            docId: 'intro',
            position: 'left',
            label: 'Documentation',
          },
          {
            href: 'https://git.psi.ch/controls-ci/std_detector_buffer',
            label: 'Gitlab Doc',
            position: 'right',
          },
        ],
      },
      prism: {
        theme: lightCodeTheme,
        darkTheme: darkCodeTheme,
      },
    }),
};

module.exports = config;
