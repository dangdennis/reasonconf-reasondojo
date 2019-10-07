

 
 /* A helper for pretty-stringifying results */
[@bs.val] external prettyStringify: ('a, Js.Nullable.t(unit), int) => string = "JSON.stringify";

let appId = "9b3ce1e5-c9e8-400c-85eb-a88de769c3e7";

module Client = {
  let authConfig =
    OneGraphAuth.createOptions(
      ~appId,
      ~oneGraphOrigin="https://serve.onegraph.io/",
      (),
    );

    let auth = OneGraphAuth.create(authConfig);

    /* Create an InMemoryCache */
    let inMemoryCache = ApolloInMemoryCache.createInMemoryCache();

    /* Create an HTTP Link */
    let httpLink =
    ApolloLinks.createHttpLink(~uri="https://serve.onegraph.com/graphql?app_id=" ++ appId, ());

    /* Dynamically generate the authorization header on every request*/
    let headerContextLink =
        ApolloLinks.createContextLink(() =>
        {
            "headers": {
            "authorization":
                switch (Js.Nullable.toOption(OneGraphAuth.accessToken(auth))) {
                | None => ""
                | Some(token) => "Bearer " ++ token##accessToken
                },
            },
        });

    let client = ReasonApollo.createApolloClient(
        ~link=ApolloLinks.from([|headerContextLink, httpLink|]),
        ~cache=inMemoryCache,
        ());
};

module UnnamedQuery1QueryConfig = [%graphql {|
  # Consider giving this query a unique, descriptive
  # name in your application as a best practice
  query unnamedQuery1 {
    rss {
      rss2Feed(url: "") {
        category
        description
        link
        pubDate
        rating
        title
        items {
          author
          link
          content
          source {
            url
            data
          }
        }
      }
    }
  }
|}
];

module UnnamedQuery1Query = ReasonApolloHooks.Query.Make(UnnamedQuery1QueryConfig);

module UnnamedQuery1 {
  [@react.component]
  let make = () => {
    open React;
    let variables = UnnamedQuery1QueryConfig.make( ())##variables;
  
    /* Both variant and records available */
    let (_simple, full) =
      UnnamedQuery1Query.use(~fetchPolicy=NetworkOnly, ~errorPolicy=All, ~variables, ());
  
    switch (full) {
    | {loading: true} => <p> {string("Loading...")} </p>
    | {data, error, refetch} =>
      let jsonify = data =>
        <pre> {string(prettyStringify(data, Js.Nullable.null, 2))} </pre>;
      let dataEl =
        data->Belt.Option.mapWithDefault(string("No data"), jsonify);
      let errorEl = error->Belt.Option.mapWithDefault(null, jsonify);
  
      <div>
        dataEl
        errorEl
        <button
          onClick=(
            (_event) =>
              switch (OneGraphAuth.findMissingAuthServices(Client.auth, error)) {
              | [] => refetch()->ignore
              | [serviceName, ..._otherServices] =>
                  serviceName
                  ->OneGraphAuth.login(Client.auth, _)
                  ->Js.Promise.then_(
                      () => OneGraphAuth.isLoggedIn(Client.auth, serviceName),
                      _,
                    )
                  ->Js.Promise.then_(
                      loginSuccess =>
                        switch (loginSuccess) {
                        | false =>
                          Js.log2(
                            "The user did not grant auth to ",
                            serviceName,
                          )
                          ->Js.Promise.resolve
                        | true =>
                          Js.log2("Successfully logged into ", serviceName);
                          refetch()->ignore->Js.Promise.resolve;
                        },
                      _,
                    )
                  ->ignore
              }
          )>
          {string("Rerun UnnamedQuery1Query")}
        </button>
      </div>;
    };
  }  
}
 
 let style = ReactDOMRe.Style.make;
    
    ReactDOMRe.renderToElementWithId(
      <ReasonApolloHooks.ApolloProvider client=Client.client>
        <App greeting="Hello, ReasonConf Dojo" />
        
      </ReasonApolloHooks.ApolloProvider>,
      "index1",
    );



