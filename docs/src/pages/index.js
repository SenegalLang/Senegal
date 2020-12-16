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
          description="Smol Programming Language">
        <div className={styles.hero}>
          <header>
            <img src={useBaseUrl('img/logo.svg')} />
            <h1>{siteConfig.title}</h1>
            <p>{siteConfig.tagline}</p>
            <div className={styles.buttons}>
              <Link to={useBaseUrl('docs/')}>Get Started</Link>
            </div>
          </header>
          <main>
            <section className={styles.section}>
              <h2>Contribution</h2>
              <ul>
                <li>Clone the repo from https://github.com/SenegalLang/Senegal</li>
                <li>Navigate to the new directory and build it by running:
                  <div class="language-shell highlighter-rouge"><div class="highlight"><pre class="highlight language-shell"><code class=" language-shell">$ cmake --build .
                  </code></pre></div>    </div>
                </li>
                <li>When ready, open and pull request and request a review from a team member.</li>
              </ul>
            </section>
          </main>
        </div>
      </Layout>
  );
}

export default Home;
