(function (angular) {
    /// TEMPLATES

    angular.module("pills.templates", []).run(["$templateCache", function($templateCache) {
        $templateCache.put("pill-services.tmpl.html", '<div watering=""></div><div another=""></div>');
        $templateCache.put("pill-services/watering.tmpl.html", [
            '<div class="col-lg-6"><div class="well">',
            '<h2>Watering Service: <span ng-class="[status.css]">{{status.label}}</span></h2>',
            '<p><a class="btn btn-info btn-lg" href="#" ng-click="onClick_Water();">Water</a></p></div></div>'
        ].join(''));

        $templateCache.put("pill-services/another.tmpl.html", [
            '<div class="col-lg-6"><div class="well">',
            '<h2>Another Service: <span ng-class="[status.css]">{{status.label}}</span></h2>',
            '<p><a class="btn btn-info btn-lg" href="#" ng-click="onClick_doSmtg();">Do something</a></p></div></div>'
        ].join(''));

        $templateCache.put("settings/settings-root.tmpl.html", [
            '<div class="col-lg-12">',
            '<h2>System Configuration Setup</h2>',
            '<p/>' +
            '<div class="row">',
                '<div class="col-md-4 wizard-steps">',
                '<div class="wizard-nav-container">',
                '<ul class="nav nav-pills nav-stacked" style="padding-bottom:30px;">',
                '<li ng-class="{\'disabled\': !vm.steps.step1.canGo()}" ui-sref-active="active"><a ui-sref=".step1" eat-click-if="vm.steps.step1.canGo()" class="wizard-nav-link"><i class="icon-chevron-right"></i>Pumps Calibration</a></li>',
                //'<li ng-class="{\'disabled\': !vm.steps.step2.canGo()}" ui-sref-active="active"><a ui-sref=".step2" eat-click-if="vm.steps.step2.canGo()" class="wizard-nav-link"><i class="icon-chevron-right"></i>Service Selection</a></li>',
                //'<li ng-class="{\'disabled\': !vm.steps.step3.canGo()}" ui-sref-active="active"><a ui-sref=".step3" eat-click-if="vm.steps.step3.canGo()" class="wizard-nav-link"><i class="icon-chevron-right"></i>Notification Schedule</a></li>',
                '</ul>',
                '</div>',
                '<div class="wizard-progress-container"><div class="progress progress-striped active"><div class="progress-bar progress-bar-success" ng-style="{ \'width\': (vm.steps.current-1)/vm.steps.total*100 + \'%\' }" style="width: 33%;"></div></div></div>',
                '</div>',
                '<div class="col-md-8" ui-view="step"></div>',
                '</div>',

                '<div class="alert alert-danger" ng-show="vm.showError()">' +
                '{{vm.errorText}}' +
                '</div>' +

            '</div>'
        ].join(''));

        $templateCache.put("settings/settings-step1.tmpl.html", [
            '<h1>Pumps calibration</h1>' +
            '<p> 1. Prepare your installation. Once everything is installed, connected and verified, press Next to open watering pump which is needed for calculating your pumps performance. </p>' +
            '<p> 2. Once a water leaks the appropriate hosepipe, press Stop button. </p>' +

            '<div class="panel panel-default"> '+
                '<div class="panel-heading">'+
                    '<h3 class="panel-title">Pumps calibration</h3>'+
                '</div>'+

                '<div class="panel panel-default"> '+
                    '<div class="panel-body">'+
                        '<div class="btn-group btn-group-xm" role="group" aria-label="...">'+
                            '<button class="btn btn-default" ng-click="vm.steps.step1.startButtonClicked(1)" ng-class="{\'disabled\': !vm.steps.step1.startButtonEnabled(1)}"> Start Pump 1</button>' +
                            '<button class="btn btn-default" ng-click="vm.steps.step1.stopButtonClicked(1)" ng-class="{\'disabled\': !vm.steps.step1.stopButtonEnabled(1)}"> Stop Pump 1</button>' +
                        '</div>' +
                        '<span class="resultLabel">Working time: {{ vm.steps.step1.workingTime(1) }}</span>' +

                        '<div class="input-group  input-group-sm inputGroup">'+
                            '<span class="input-group-addon" id="basic-addon1">Water pumped (ml)</span>'+
                            '<input  name="input" ng-disabled="!vm.steps.step1.inputEnabled(1)" type="number" min="0" max="10000" required class="form-control waterAmountInput" id="exampleInputName2" ng-model="vm.steps.step1.pumpedMlCount[0]"/>' +
                            '<span class="error" ng-show="AmountInputForm.input.$error.number"> Not valid number!</span>'  +
                        '</div>'+
                    '</div>  '+

                '<div class="panel panel-default"> '+
                    '<div class="panel-body">'+
                        '<div class="btn-group btn-group-xm" role="group" aria-label="...">'+
                            '<button class="btn btn-default" ng-click="vm.steps.step1.startButtonClicked(2)" ng-class="{\'disabled\': !vm.steps.step1.startButtonEnabled(2)}"> Start Pump 2</button>' +
                            '<button class="btn btn-default" ng-click="vm.steps.step1.stopButtonClicked(2)" ng-class="{\'disabled\': !vm.steps.step1.stopButtonEnabled(2)}"> Stop Pump 2</button>' +
                        '</div>' +
                        '<span class="resultLabel">Working time: {{ vm.steps.step1.workingTime(2) }}</span>' +

                        '<div class="input-group  input-group-sm inputGroup">'+
                            '<span class="input-group-addon" id="basic-addon1">Water pumped (ml)</span>'+
                            '<input  name="input" ng-disabled="!vm.steps.step1.inputEnabled(1)" type="number" min="0" max="10000" required class="form-control waterAmountInput" id="exampleInputName2" ng-model="vm.steps.step1.pumpedMlCount[1]"/>' +
                            '<span class="error" ng-show="AmountInputForm.input.$error.number"> Not valid number!</span>'  +
                        '</div>'+
                    '</div>  '+
                '</div>  '+
            '</div>  '+

            //'<button class="btn btn-danger" ng-click="vm.steps.step1.warmup()" ng-class="{\'disabled\': !vm.steps.step1.canGoToStrep2()}"> Coo </button>' +
            '<a role="button" ng-disabled="!vm.steps.step1.completionCheck()" eat-click-if="vm.steps.step2.canGo()" ui-sref="^.step2" type="button" class="btn btn-success" ng-click="vm.steps.step1.completedCallback()">Next</button>'
        ].join(''));

        //$templateCache.put("settings/settings-step2.tmpl.html", [
        //    '<h1>Step 2</h1>',
        //    '<a role="button" eat-click-if="vm.steps.step1.canGo()" ui-sref="^.step1" type="button" class="btn btn-success">Previous</button>',
        //    '<a role="button" ng-disabled="!vm.steps.step2.completionCheck()" eat-click-if="vm.steps.step3.canGo()" ui-sref="^.step3" type="button" class="btn btn-success" ng-click="vm.steps.step2.completedCallback()">Next</button>'
        //].join(''));
        //
        //$templateCache.put("settings/settings-step3.tmpl.html", [
        //    '<h1>Step 3</h1>',
        //    '<a role="button" eat-click-if="vm.steps.step2.canGo()" ui-sref="^.step2" type="button" class="btn btn-success">Previous</button>',
        //    '<a role="button" ng-disabled="!vm.steps.step3.completionCheck()" ui-sref="app" type="button" class="btn btn-success" ng-click="vm.steps.step3.completedCallback()">Finish</button>'
        //].join(''));
    }]);

    /// CONFIG

    angular.module('pills', [
        'pills.dependency', 'pills.config', 'pills.router', 'pills.controllers', 'pills.templates',
        'pills.watering', 'pills.another', 'pills.settings', 'pills.header'
    ]);

    angular.module('pills.dependency', ['ui.router', 'ngResource', 'eatClickIf']);

    angular.module('pills').config(['$locationProvider', '$httpProvider', '$urlRouterProvider', function($locationProvider, $httpProvider, $urlRouterProvider){
        $httpProvider.defaults.useXDomain = true;
        delete $httpProvider.defaults.headers.common['X-Requested-With'];
        $httpProvider.defaults.headers.post['Content-Type'] = 'application/json;charset=utf-8';
        $httpProvider.defaults.useXDomain = true;
        $locationProvider.html5Mode(false);
        $urlRouterProvider.otherwise('/');
    }]);

    angular.module('pills.config', []).constant('config', {
        api: "http://127.0.0.1:9999/",
        calibrationServiceApi: "http://127.0.0.1:4567/",
        statuses: {
            ready:{value:0, label:'Ready', css:"text-success"},
            working:{value:1, label:'Working..', css:"text-warning"},
            error:{value:2, label:'Error', css:"text-danger"}
        }
    });

    angular.module('pills.router', []).config(['$stateProvider', function ($stateProvider) {
        $stateProvider.state('app', {
            url: '/',
            views: {
                '@': {
                    templateUrl: 'pill-services.tmpl.html',
                    controller: 'PillServicesController'
                }
            }
        }).state('wizard', {
            url: '/settings',
            views: {
                '@': {
                    templateUrl: 'settings/settings-root.tmpl.html',
                    controller: 'SettingsController as vm'
                }
            }
        }).state('wizard.step1', {
            url: '/step1',
            views: {
                'step': {
                    templateUrl: 'settings/settings-step1.tmpl.html',
                }
            },
            data: {
                step: 1
            }});
        //}).state('wizard.step2', {
        //    url: '/step2',
        //    views: {
        //        'step': {
        //            templateUrl: 'settings/settings-step2.tmpl.html',
        //        }
        //    },
        //    data: {
        //        step: 2
        //    }
        //}).state('wizard.step3', {
        //    url: '/step3',
        //    views: {
        //        'step': {
        //            templateUrl: 'settings/settings-step3.tmpl.html',
        //        }
        //    },
        //    data: {
        //        step: 3
        //    }
        //});
    }]);

    angular.module('pills.controllers', []).controller('PillServicesController', ['$scope', function PillServicesController($scope) {
        var _this = this;
    }]);

    angular.element(document).ready(function () {
        angular.bootstrap(document, ['pills'], {
            strictDi: true
        });
    });

    /// SETTINGS
    angular.module('pills.settings', ['pills.settings.controller']);
    angular.module('pills.settings.controller', ['pills.calibration']).controller('SettingsController', ['$scope', '$state', '$rootScope', 'CalibrationService',
        function SettingsController($scope, $state, $rootScope, CalibrationService) {
            var _this = this;


            this.settings = {
                value1: '',
                value2: '',
                value3: '',
                value4: ''
            };

            //var resultId = null;

            this.errorText = "";
            this.showError = function () {
                return _this.errorText !== "";
            };

            var stateChangeCancelation = $rootScope.$on('$stateChangeStart', function (event, toState, toParams, fromState, fromParams) {
                if (toState.name.indexOf('wizard') == -1) {
                    stateChangeCancelation();
                } else {
                    if (angular.isUndefined(toState.data) || angular.isUndefined(toState.data.step)) {
                        $state.go('wizard.step1');
                        return;
                    }
                    if (!(_this.steps['step' + toState.data.step].canGo())) {
                        event.preventDefault();
                        return;
                    }
                    _this.steps.current = toState.data.step;
                }
            });

            if (angular.isUndefined($state.$current.data) || angular.isUndefined($state.$current.data.step)) {
                $state.go('wizard.step1', { reload: true });
                return;
            }

            var resultId = null;
            var _workingTime = '---';
            var firstStepReady = false;

            var pumpData = {
                pump1: {
                    workingTime: null,
                    state: 'stopped',
                    invervalTimer: null
                },
                pump2: {
                    workingTime: null,
                    state: 'stopped',
                    invervalTimer: null
                }
            };

            var progressInterval = null;

            this.steps =
            {
                current: $state.$current.data.step || 1,
                total: 3,




                step1: {
                    pumpedMlCount: [0, 0],

                    inputEnabled: function() {
                        return firstStepReady;
                    },

                    canGo: function() {
                        return true;
                    },

                    completionCheck: function() {
                        return (_this.steps.step1.pumpedMlCount[0] > 0) && (_this.steps.step1.pumpedMlCount[1] > 0);
                    },

                    completedCallback: function() {
                        _this.steps.step1.completed = true;
                    },

                    startButtonClicked: function(pumpNumber) {
                        if (!CalibrationService.isConnectionOk()) {
                            console.log('Failed starting the pump. Backend/Hardware service connection problem');
                            return;
                        }
                        pumpData["pump"+pumpNumber].state = 'started';

                        firstStepReady = false;
                        _this.steps.step1.pumpedMlCount[pumpNumber-1] = 0;

                        var charIndex = 0;
                        pumpData["pump"+pumpNumber].invervalTimer = setInterval(function () {
                            $scope.$apply(function() {
                                pumpData["pump"+pumpNumber].workingTime = ['|', '/', '-', '\\'][charIndex % 4];
                                //console.log('_workingTime: ' + pumpData["pump"+pumpNumber].workingTime);
                            });
                            charIndex += 1;
                            }, 75);
                        CalibrationService.startPump(pumpNumber).then(function (data) {
                        }, function (error) {
                            _this.errorText = "Failed starting pump: " + JSON.stringify(error);
                        });
                    },

                    stopButtonClicked: function(pumpNumber) {
                        if (!CalibrationService.isConnectionOk()) {
                            console.log('Failed stopping the pump. Backend/Hardware service connection problem');
                            return;
                        }
                        CalibrationService.stopPump(pumpNumber).then(function (data) {
                            if (pumpData["pump"+pumpNumber].invervalTimer !== null)
                                clearInterval(pumpData["pump"+pumpNumber].invervalTimer);
                            pumpData["pump"+pumpNumber].workingTime = data.result.workingTimeMs;
                            pumpData["pump"+pumpNumber].state = 'stopped';
                            firstStepReady = true;

                        }, function (error) {
                            _this.errorText = "Failed stopping pump: " + JSON.stringify(error);
                        });

                        _this.errorText = "";
                    },

                    stopButtonEnabled: function(pumpNumber) {
                        return pumpData["pump"+pumpNumber].state === 'started';
                    },
                    startButtonEnabled: function(pumpNumber) {
                        return pumpData["pump"+pumpNumber].state === 'stopped' && CalibrationService.isConnectionOk();
                    },

                    canGoToStrep2: function() {
                        return true;
                    },

                    workingTime: function(number){return pumpData['pump' + number].workingTime === null ? '---' : pumpData['pump' + number].workingTime; },

                    completed: false
                },
                step2: {
                    canGo: function() {
                        return _this.steps.step1.completed;
                    },
                    completionCheck: function() {
                        return true;
                    },
                    completedCallback: function() {
                        _this.steps.step2.completed = true;
                    },
                    completed: false
                },
                step3: {
                    canGo: function() {
                        return _this.steps.step2.completed;
                    },
                    completionCheck: function() {
                        return true;
                    },
                    completedCallback: function() {
                        _this.steps.step3.completed = true;
                    },
                    completed: false
                }
            };

            if (!(_this.steps['step' + _this.steps.current].canGo())) {
                $state.go('wizard.step1', { reload: true });
                return;
            }
    }]);

    // ------------------------------------------------------------------------------------------------------------------------------------
    // MenuController
    // ------------------------------------------------------------------------------------------------------------------------------------
    angular.module('pills.header', []).controller('MenuController',['CalibrationService', function (CalibrationService) {
        var _this= this;

        this.isBackEndOnline = function (){
            return CalibrationService.getStatus().status === 0;
        };
        this.isHardwareOnline = function (){
            return CalibrationService.getStatus().hardStatus === 0;
        };
    }]);

    // ------------------------------------------------------------------------------------------------------------------------------------
    // Callibration service
    // ------------------------------------------------------------------------------------------------------------------------------------
    angular.module('pills.calibration', []).factory('CalibrationService', ['$q', '$resource', 'config', function ($q, $resource, config) {

        var hardwareServiceResource = $resource(buildUrl('pumpControl/:action'), null, {
            startPump: { method: 'POST', params: { action:'start'} },
            stopPump: { method: 'POST', params: { action: 'stop'} }
        });

        var statusResource = $resource(buildUrl('status'), null, {
            getStatus: { method: 'GET' }
        });

        var stopResult = $resource(buildUrl('stop_result/:resultId'), { resultId:'@resultId' }, {
            getResult: { method: 'GET' }
        });


        var currentStatus = {status: 0, hardStatus: 0};

        // Status poller
        setInterval(function () {
            statusResource.getStatus().$promise.then(function (data) {
                console.log("Got a status: " + JSON.stringify(data));
                currentStatus = data;
            }, function (error) {
                console.log("Failed getting status");
                currentStatus = {status: 1, hardStatus: 1};
            });

        }, 500);

        var _public = {
            startPump: function(pumpId) {
                return hardwareServiceResource.startPump({pumpId: pumpId}).$promise;
            },
            stopPump: function (pumpId) {
                var deferred = $q.defer();
                 hardwareServiceResource.stopPump({pumpId: pumpId}).$promise.then(function (data) {
                    resultId = data.resultId;
                    //// start result polling
                    var pollingFunction = function () {
                        _public.getStopResult(data.resultId).then(function (response) {
                            // got stop result, check it
                            if (response.status === 'not_ready') {
                                console.log('restart poller..');
                                setTimeout(pollingFunction, 100);
                            } else {
                                deferred.resolve(response);
                            }
                        }, function (error) {
                            // failed getting stop result
                            console.log('restart poller due to error..');
                            setTimeout(pollingFunction, 100);
                        });
                    };

                    // Try to get results after 100 msecs after request
                    setTimeout(pollingFunction, 0);

                }, function (error) {
                     deferred.reject(error); // (will send control to error part of .then clause)
                });
                return deferred.promise;
            },
            getStatus: function() {
                return currentStatus;
            },
            isConnectionOk: function() {
                return currentStatus.status === 0 && currentStatus.hardStatus === 0;
            },
            getStopResult: function (resultId) {
                return stopResult.getResult({}, { resultId: resultId }).$promise;
            }
        };

        return _public;

        function buildUrl(resourceUrl, domain) {
            if (resourceUrl.lastIndexOf('/') !== resourceUrl.length - 1) {
                resourceUrl += "/";
            }

            return (domain ? domain : config.calibrationServiceApi) + resourceUrl;
        }
    }]);

    /// WATERING

    angular.module('pills.watering', ['pills.watering.resource', 'pills.watering.directive']);

    function WateringApiService($q, $resource, $timeout, config){
        var api = {};

        var waterResource = $resource(buildUrl('water/:action'), null, {
            water: { method: 'POST' },
            status: { method: 'GET', params: { action:'status'}, isArray: false }
        });

        api.water = function water(data) {
            var q = $q.defer();
            $timeout(function(){ q.resolve('OK');}, 1000);
            return q.promise;
            //return waterResource.water({}, data).$promise;
        };

        api.getStatus = function getStatus () {
            var q = $q.defer();
            $timeout(function(){ q.resolve('OK');}, 1000);
            return q.promise;
            return waterResource.status({}).$promise;
        };

        return api;

        ////////////////////////

        function buildUrl(resourceUrl, domain) {
            if (resourceUrl.lastIndexOf('/') !== resourceUrl.length - 1) {
                resourceUrl += "/";
            }

            return (domain ? domain : config.api) + resourceUrl;
        }
    }

    angular.module('pills.watering.resource', []).factory('WateringApiService', ['$q', '$resource', '$timeout', 'config', WateringApiService]);

    angular.module('pills.watering.directive', []).directive('watering', ['WateringApiService', 'config', function (WateringApiService, config){
        return {
            resrict: 'A',
            replace: true,
            scope:{},
            templateUrl: 'pill-services/watering.tmpl.html',
            controller: ['$scope', function ($scope) {
                $scope.status =  config.statuses.ready;
                // WateringApiService.status().then(function(data){
                // 	$scope.status = data;
                // 	console.log('Watering...');
                // }, function (error){
                // 	scope.status = config.statuses.error;
                // 	console.error('Error watering: ');
                // 	console.error(error);
                // });
            }],
            link: function (scope){
                scope.onClick_Water = function(){
                    scope.status = config.statuses.working;
                    WateringApiService.water().then(function(){
                        scope.status = config.statuses.ready;
                        console.log('Watering...');
                    }, function (error){
                        scope.status = config.statuses.error;
                        console.error('Error watering: ');
                        console.error(error);
                    });
                };
            }
        }
    }]);

    /// AnotherService

    angular.module('pills.another', ['pills.another.resource', 'pills.another.directive']);

    function AnotherApiService($q, $resource, config){
        var api = {};

        var anotherResource = $resource(buildUrl('another/:action'), null, {
            doSmtg: { method: 'POST' },
            status: { method: 'GET', params: { action:'status'}, isArray: false }
        });

        api.doSmtg = function doSmtg(data) {
            return $q.when(true);
            //return anotherResource.water({}, data).$promise;
        };

        api.getStatus = function getStatus () {
            return $q.when(true);
            //return anotherResource.status({}).$promise;
        };

        return api;

        ////////////////////////

        function buildUrl(resourceUrl, domain) {
            if (resourceUrl.lastIndexOf('/') !== resourceUrl.length - 1) {
                resourceUrl += "/";
            }
            return (domain ? domain : config.api) + resourceUrl;
        }
    }

    angular.module('pills.another.resource', []).factory('AnotherApiService', ['$q', '$resource', 'config', AnotherApiService]);

    angular.module('pills.another.directive', []).directive('another', ['AnotherApiService', 'config', function (AnotherApiService){
        return {
            resrict: 'A',
            replace: true,
            scope:{},
            templateUrl: 'pill-services/another.tmpl.html',
            controller: ['$scope', 'config', function ($scope, config) {
                $scope.status = config.statuses.error;
            }],
            link: function (scope){
                scope.onClick_doSmtg = function(){
                    AnotherApiService.doSmtg().then(function(){
                        console.log('Another service Do something...');
                    }, function (error){
                        console.error('Error doing something: ');
                        console.error(error);
                    });
                };
            }
        }
    }]);

    /// EatClickIf
    angular.module('eatClickIf', []).directive('eatClickIf', [
        '$parse', '$rootScope', function($parse, $rootScope) {
            return {
                priority: 100,
                restrict: 'A',
                compile: function($element, attr) {
                    var fn = $parse(attr.eatClickIf);
                    return {
                        pre: function link(scope, element) {
                            var eventName = 'click';
                            element.on(eventName, function(event) {
                                var callback = function() {
                                    if (!fn(scope, { $event: event })) {
                                        event.stopImmediatePropagation();
                                        event.preventDefault();
                                        event.stopPropagation();
                                        return false;
                                    }
                                };
                                if ($rootScope.$$phase) {
                                    scope.$evalAsync(callback);
                                } else {
                                    scope.$apply(callback);
                                }
                            });
                        }
                    }
                }
            }
        }
    ]);
})(angular);