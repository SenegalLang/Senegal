
import React from 'react';
import ComponentCreator from '@docusaurus/ComponentCreator';
export default [
{
  path: '/',
  component: ComponentCreator('/','d7d'),
  exact: true,
},
{
  path: '/community',
  component: ComponentCreator('/community','ee3'),
  
  routes: [
{
  path: '/community/support',
  component: ComponentCreator('/community/support','361'),
  exact: true,
},
]
},
{
  path: '/docs',
  component: ComponentCreator('/docs','873'),
  
  routes: [
{
  path: '/docs/',
  component: ComponentCreator('/docs/','76a'),
  exact: true,
},
{
  path: '/docs/tour',
  component: ComponentCreator('/docs/tour','446'),
  exact: true,
},
]
},
{
  path: '*',
  component: ComponentCreator('*')
}
];
