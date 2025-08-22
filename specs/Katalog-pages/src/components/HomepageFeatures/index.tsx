import clsx from 'clsx';
import Heading from '@theme/Heading';
import styles from './styles.module.css';
import Translate, {translate} from '@docusaurus/Translate';

import deviceImage from '@site/static/img/features.png';
import platformImage from '@site/static/img/linux_windows.png';
import openSourceImage from '@site/static/img/qt-seeklogo.png';

// import translations from '@site/i18n/fr/HomepageFeatures.json';
// console.log(translations);

//import useDocusaurusContext from '@docusaurus/useDocusaurusContext';

type FeatureItem = {
  title: string;
  image: string;
  description: JSX.Element;
};

const FeatureList: FeatureItem[] = [
  {
    title: translate({
      id: 'feature.deviceManagement.title',
      message: 'Devices and Files management capabilities',
      description: 'The title of the first feature'
    }),
    image: deviceImage,
    description: translate({
      id: 'feature.deviceManagement.description',
      message: 'Katalog brings extensive file search and comparison capabilities and management of devices storing data.',
      description: 'The first part of the description of the first feature'
    })
  },
  {
    title: translate({
      id: 'feature.multiPlatform.title',
      message: 'Multi desktop platforms, multi languages',
      description: 'The title of the first feature'
    }),
    image: platformImage,
    description: translate({
      id: 'feature.multiPlatform.description',
      message: 'Katalog is currently available for Linux and Windows, and in many languages.',
      description: 'The first part of the description of the first feature'
    })
  },
  {
    title: translate({
      id: 'feature.openSource.title',
      message: 'Open source',
      description: 'The title of the first feature'
    }),
    image: openSourceImage,
    description: translate({
      id: 'feature.openSource.description',
      message: 'Katalog is designed with Qt and its source code is available on GitHub.',
      description: 'The first part of the description of the first feature'
    })
  }
];

function Feature({title, image, description}: FeatureItem) {
  return (
    <div className={clsx('col col--4')}>
      <div className="text--center">
        <img src={image} className={styles.featureImage} alt={title} height="100" />
      </div>
      <div className="text--center padding-horiz--md">
        <Heading as="h3">{title}</Heading>
        <p>{description}</p>
      </div>
    </div>
  );
}

export default function HomepageFeatures(): JSX.Element {
  return (
    <section className={styles.features}>
    <div className="container">
    <div className="row">
    {FeatureList.map((props, idx) => (
      <Feature key={idx} {...props} />
    ))}
    </div>
    </div>
    </section>
  );
}

