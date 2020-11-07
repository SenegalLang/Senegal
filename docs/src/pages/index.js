import React from 'react';
import classnames from 'classnames';
import Layout from '@theme/Layout';
import Link from '@docusaurus/Link';
import useDocusaurusContext from '@docusaurus/useDocusaurusContext';
import useBaseUrl from '@docusaurus/useBaseUrl';

import styles from './styles.module.css';

function Home() {
  const context = useDocusaurusContext();
  const {siteConfig = {}} = context;
  return (
    <Layout
      title={siteConfig.title}
      description="Senegal Programming Language">
      <div className={styles.hero}>
        <header>
          <img src={useBaseUrl('img/logo.svg')} />
          <h1>{siteConfig.title}</h1>
          <div className={styles.buttons}>
            <Link to={useBaseUrl('docs/')}>Get Started</Link>
          </div>
        </header>
      </div>
    </Layout>
  );
}

export default Home;
