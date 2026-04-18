import clsx from 'clsx';
import Link from '@docusaurus/Link';
import useDocusaurusContext from '@docusaurus/useDocusaurusContext';
import Layout from '@theme/Layout';
import HomepageFeatures from '@site/src/components/HomepageFeatures';
import Heading from '@theme/Heading';
import React from 'react';
import Translate from '@docusaurus/Translate';

import styles from './index.module.css';

function HomepageHeader() {
  const {siteConfig} = useDocusaurusContext();
  return (
    <header className={clsx('hero hero--primary', styles.heroBanner)}>
      <div className="container">
        <img src="img/Banner_2.11.png" width="900"></img>
        <div className={styles.buttons}>
          <Link
            className="button button--secondary button--lg"
            to="https://flathub.org/en/apps/io.github.stephanecouturier.Katalog">
            <Translate id="homepage.header.downloadButton" description="Download Katalog from sourceforge">
            Install (Flatpak)
            </Translate>&nbsp;&#10515;
          </Link>
          <Link
            className="button button--secondary button--lg"
            to="https://sourceforge.net/projects/katalogg/files/latest/download">
            <Translate id="homepage.header.downloadButton" description="Download Katalog from sourceforge">
              Download
            </Translate>&nbsp;&#10515;
          </Link>
          <Link
            className="button button--secondary button--lg"
            to="/docs/tutorial">
            <Translate id="homepage.header.tutorialButton" description="Get started with Katalog in 5 minutes">
            Tutorial - 5min
            </Translate> ⏱️
          </Link>
          <Link
            className="button button--secondary button--lg"
            to="/docs/Overview">
            <Translate id="homepage.header.overviewButton" description="Discover Katalog's features'">
            Features Overview
            </Translate>
          </Link>
        </div>
      </div>
    </header>
  );
}

export default function Home(): JSX.Element {
  const {siteConfig} = useDocusaurusContext();
  return (
    <Layout
      title={`${siteConfig.title}`}
      description="Catalog and Search Files">
      <HomepageHeader />
      <main>
        <HomepageFeatures />
      </main>
    </Layout>
  );
}

