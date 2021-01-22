module.exports = {
    SenegalLang: [
      {
        type: 'category',
        label: 'Senegal Docs',
        items: [
        'overview',
        'tour',
        'concurrency',
        'cimport',
        'enhance',
        'operators',
        'pipeline',
            {
                Core: [
                    'Core/bool',
                    'Core/coroutine',
                    'Core/list',
                    'Core/map',
                    'Core/num',
                    'Core/string'
                ]
            },
        ],
      },
        {
         type: 'category',
         label: 'Contribute',
         items: [
            'Contribute/style',
            'Contribute/tree_hygiene',
            'Contribute/contributing',
         ],
        },
    ]
};
